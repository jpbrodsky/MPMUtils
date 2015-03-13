#include "TSQLHelper.hh"
#include <stdlib.h>
#include <unistd.h>
#include "StringManip.hh"
#include "SMExcept.hh"

TSQLHelper::TSQLHelper(const std::string& dbnm,
                     const std::string& dbAddress,
                     const std::string& dbUser,
                     const std::string& dbPass,
                     unsigned int port,
                     unsigned int ntries): db(NULL), res(NULL), dbName(dbnm) {
                         
    string dbAddressFull = "mysql://"+dbAddress+":"+to_str(port)+"/"+dbnm;
    while(!db) {
        ntries--;
        db = TSQLServer::Connect(dbAddressFull.c_str(),dbUser.c_str(),dbPass.c_str());
        if(!db) {
            if(!ntries || IGNORE_DEAD_DB)
                break;
            printf("** DB Connection %s@%s failed... retrying...\n",dbUser.c_str(),dbAddressFull.c_str());
            sleep(2);
        } else {
            printf("Connected to DB server: %s\n", db->ServerInfo());
            return;
        }
    }
    SMExcept e("DBConnectFail");
    e.insert("dbAddress",dbAddressFull);
    e.insert("dbUser",dbUser);
    throw(e);
}

void TSQLHelper::execute() {
    if(res) delete(res);
    res = NULL;
    if(!db->Exec(query)) {
        SMExcept e("DBExecFail");
        e.insert("query",query);
        throw(e);
    }
}

void TSQLHelper::Query() { 
    if(!db) {
        res = NULL;
    } else {
        if(res) delete(res);
        res = db->Query(query);
        if(db->GetErrorCode()) {
            SMExcept e("DBQueryFail");
            e.insert("query",query);
            throw(e);
        }
    }
}

string TSQLHelper::fieldAsString(TSQLRow* row, unsigned int fieldnum, const std::string& dflt) {
    smassert(row);
    const char* s = row->GetField(fieldnum);
    isNullResult = !s;
    if(isNullResult)
        return dflt;
    return std::string(s);
}

int TSQLHelper::fieldAsInt(TSQLRow* row, unsigned int fieldnum, int dflt) {
    string s = fieldAsString(row,fieldnum);
    if(isNullResult)
        return dflt;
    return atoi(s.c_str());
}

float TSQLHelper::fieldAsFloat(TSQLRow* row, unsigned int fieldnum, float dflt) {
    string s = fieldAsString(row,fieldnum);
    if(isNullResult)
        return dflt;
    return atof(s.c_str());
}

int TSQLHelper::getInsertID() {
    sprintf(query,"SELECT LAST_INSERT_ID()");
    Query();
    TSQLRow* r = getFirst();
    if(!r)
        throw(SMExcept("failedInsert"));
    int rid = fieldAsInt(r,0);
    delete(r);
    if(!rid)
        throw(SMExcept("failedInsert"));
    return rid;
}

void TSQLHelper::printResult() {
    TSQLRow* row;
    while( (row = res->Next()) ) {
        printf("----------------\n");
        for(int i=0; i<res->GetFieldCount(); i++)
            printf("%s:\t%s\n",res->GetFieldName(i),fieldAsString(row,i,"NULL").c_str());
        delete(row);
    }
}

TSQLRow* TSQLHelper::getFirst() {
    Query();
    if(!res)
        return NULL;
    return res->Next();
}

string sm2insert(const Stringmap& m) {
    string svars = "(";
    string svals = "VALUES (";
    for(map<std::string,std::string>::const_iterator it = m.dat.begin(); it != m.dat.end(); it++) {
        if(it != m.dat.begin()) {
            svars += ",";
            svals += ",";
        }
        svars += std::string("`")+it->first+"`";
        svals += it->second;
    }
    return svars+") "+svals+")";
}

