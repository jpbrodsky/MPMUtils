/// \file SQLite_Helper.cc
// This file was produced under the employ of the United States Government,
// and is consequently in the PUBLIC DOMAIN, free from all provisions of
// US Copyright Law (per USC Title 17, Section 105).
//
// -- Michael P. Mendenhall, 2015

#include "SQLite_Helper.hh"
#include "SMExcept.hh"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

using std::pair;

/// callback function to display SQLite3 errors
void errorLogCallback(void*, int iErrCode, const char* zMsg){
    fprintf(stderr, "SQL error (%d): %s\n", iErrCode, zMsg);
}

bool SQLite_Helper::errlog_configured = false;

SQLite_Helper::SQLite_Helper(const string& dbname) {
    if(!errlog_configured) {
        sqlite3_config(SQLITE_CONFIG_LOG, &errorLogCallback, NULL);
        errlog_configured = true;
    }
    printf("Opening SQLite3 DB '%s'...\n",dbname.c_str());
    int err = sqlite3_open(dbname.c_str(), &db);
    if(err) {
        SMExcept e("failed_db_open");
        e.insert("dbname",dbname);
        e.insert("err",err);
        e.insert("message",sqlite3_errmsg(db));
        sqlite3_close(db);
        db = NULL;
        throw e;
    }
}

SQLite_Helper::~SQLite_Helper() {
    if(db) {
        for(auto const& kv: statements) sqlite3_finalize(kv.second);
        sqlite3_close(db);
    }
}

int SQLite_Helper::setQuery(const char* qry, sqlite3_stmt*& stmt) {
    int rc;
    while((rc = sqlite3_prepare_v2(db, qry, strlen(qry), &stmt, NULL)) == SQLITE_BUSY) {
        printf("Waiting for DB retry preparing statement...\n");
        fflush(stdout);
        usleep(500000+(rand()%500000));
    }
    if(rc != SQLITE_OK) {
        SMExcept e("failed_query");
        e.insert("err",rc);
        e.insert("message",sqlite3_errmsg(db));
        throw(e);
    }
    return rc;
}

sqlite3_stmt* SQLite_Helper::loadStatement(const string& qry) {
    auto it = statements.find(qry);
    if(it != statements.end()) return it->second;
    sqlite3_stmt* stmt = NULL;
    setQuery(qry.c_str(), stmt);
    statements.insert(pair<string,sqlite3_stmt*>(qry,stmt));
    return stmt;
}

int SQLite_Helper::busyRetry(sqlite3_stmt*& stmt) {
    int rc;
    while((rc = sqlite3_step(stmt)) == SQLITE_BUSY) {
        printf("Waiting for DB retry executing statement...\n");
        fflush(stdout);
        usleep(500000+(rand()%500000));
        sqlite3_reset(stmt);
    }
    return rc;
}

int SQLite_Helper::exec(const string& qry, bool checkOK) {
    sqlite3_stmt* stmt = loadStatement(qry);
    int rc = busyRetry(stmt);
    sqlite3_reset(stmt);
    if(checkOK && !(rc == SQLITE_OK || rc == SQLITE_DONE)) {
        SMExcept e("failed_exec");
        e.insert("err",rc);
        e.insert("message",sqlite3_errmsg(db));
        throw(e);
    }
    return rc;
}

void SQLite_Helper::getVecBlob(vector<double>& v, sqlite3_stmt*& stmt, int col) {
    const void* vdat = sqlite3_column_blob(stmt, col);
    if(!vdat) { v.clear(); return; }
    int nbytes = sqlite3_column_bytes(stmt, col);
    v.resize(nbytes/sizeof(double));
    memcpy(v.data(), vdat, nbytes);
}

int SQLite_Helper::bindVecBlob(sqlite3_stmt*& stmt, int i, const vector<double>& v) {
    return sqlite3_bind_blob(stmt, i, v.data(), v.size()*sizeof(double), NULL);
}
