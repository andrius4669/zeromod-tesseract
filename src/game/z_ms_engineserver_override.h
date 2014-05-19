#ifndef Z_MS_ENGINESERVER_OVERRIDE_H
#define Z_MS_ENGINESERVER_OVERRIDE_H 1

ENetAddress serveraddress = { ENET_HOST_ANY, ENET_PORT_ANY };

struct msinfo
{
    string mastername;
    int masterport;
    string masterauth;
    ENetSocket mastersock;
    ENetAddress masteraddress;
    int lastupdatemaster, lastconnectmaster, masterconnecting, masterconnected;
    vector<char> masterout, masterin;
    int masteroutpos, masterinpos;
    bool allowupdatemaster;
    int masternum;
    
    msinfo(): masterport(server::masterport()), mastersock(ENET_SOCKET_NULL),
        lastupdatemaster(0), lastconnectmaster(0), masterconnecting(0), masterconnected(0),
        masteroutpos(0), masterinpos(0), allowupdatemaster(true), masternum(-1)
    {
        copystring(mastername, server::defaultmaster());
        masterauth[0] = '\0';
        masteraddress.host = ENET_HOST_ANY;
        masteraddress.port = ENET_PORT_ANY;
    }
    ~msinfo() { disconnectmaster(); }
    
    void disconnectmaster()
    {
        if(mastersock != ENET_SOCKET_NULL)
        {
            server::masterdisconnected(masternum);
            enet_socket_destroy(mastersock);
            mastersock = ENET_SOCKET_NULL;
        }
        
        masterout.setsize(0);
        masterin.setsize(0);
        masteroutpos = masterinpos = 0;
        
        masteraddress.host = ENET_HOST_ANY;
        masteraddress.port = ENET_PORT_ANY;
        
        lastupdatemaster = masterconnecting = masterconnected = 0;
    }
    
    ENetSocket connectmaster(bool wait)
    {
        if(!mastername[0]) return ENET_SOCKET_NULL;
        if(masteraddress.host == ENET_HOST_ANY)
        {
            if(isdedicatedserver()) logoutf("looking up %s...", mastername);
            masteraddress.port = masterport;
            if(!resolverwait(mastername, &masteraddress)) return ENET_SOCKET_NULL;
        }
        ENetSocket sock = enet_socket_create(ENET_SOCKET_TYPE_STREAM);
        if(sock == ENET_SOCKET_NULL)
        {
            if(isdedicatedserver()) logoutf("could not open master server socket");
            return ENET_SOCKET_NULL;
        }
        if(wait || serveraddress.host == ENET_HOST_ANY || !enet_socket_bind(sock, &serveraddress))
        {
            enet_socket_set_option(sock, ENET_SOCKOPT_NONBLOCK, 1);
            if(wait)
            {
                if(!connectwithtimeout(sock, mastername, masteraddress)) return sock;
            }
            else if(!enet_socket_connect(sock, &masteraddress)) return sock;
        }
        enet_socket_destroy(sock);
        if(isdedicatedserver()) logoutf("could not connect to master server (%s)", mastername);
        return ENET_SOCKET_NULL;
    }
    
    bool requestmaster(const char *req)
    {
        if(mastersock == ENET_SOCKET_NULL)
        {
            mastersock = connectmaster(false);
            if(mastersock == ENET_SOCKET_NULL) return false;
            lastconnectmaster = masterconnecting = totalmillis ? totalmillis : 1;
        }
        
        if(masterout.length() >= 4096) return false;
        
        masterout.put(req, strlen(req));
        return true;
    }
    
    bool requestmasterf(const char *fmt, ...)
    {
        defvformatstring(req, fmt, fmt);
        return requestmaster(req);
    }
    
    void processmasterinput()
    {
        if(masterinpos >= masterin.length()) return;
        
        char *input = &masterin[masterinpos], *end = (char *)memchr(input, '\n', masterin.length() - masterinpos);
        while(end)
        {
            *end++ = '\0';
            
            const char *args = input;
            while(args < end && !iscubespace(*args)) args++;
            int cmdlen = args - input;
            while(args < end && iscubespace(*args)) args++;
            
            if(!strncmp(input, "failreg", cmdlen))
                conoutf(CON_ERROR, "master server (%s) registration failed: %s", mastername, args);
            else if(!strncmp(input, "succreg", cmdlen))
                conoutf("master server (%s) registration succeeded", mastername);
            else server::processmasterinput(masternum, input, cmdlen, args);
            
            masterinpos = end - masterin.getbuf();
            input = end;
            end = (char *)memchr(input, '\n', masterin.length() - masterinpos);
        }
        
        if(masterinpos >= masterin.length())
        {
            masterin.setsize(0);
            masterinpos = 0;
        }
    }
    
    void flushmasteroutput()
    {
        if(masterconnecting && totalmillis - masterconnecting >= 60000)
        {
            logoutf("could not connect to master server (%s)", mastername);
            disconnectmaster();
        }
        if(masterout.empty() || !masterconnected) return;
        
        ENetBuffer buf;
        buf.data = &masterout[masteroutpos];
        buf.dataLength = masterout.length() - masteroutpos;
        int sent = enet_socket_send(mastersock, NULL, &buf, 1);
        if(sent >= 0)
        {
            masteroutpos += sent;
            if(masteroutpos >= masterout.length())
            {
                masterout.setsize(0);
                masteroutpos = 0;
            }
        }
        else disconnectmaster();
    }
    
    void flushmasterinput()
    {
        if(masterin.length() >= masterin.capacity())
            masterin.reserve(4096);
        
        ENetBuffer buf;
        buf.data = masterin.getbuf() + masterin.length();
        buf.dataLength = masterin.capacity() - masterin.length();
        int recv = enet_socket_receive(mastersock, NULL, &buf, 1);
        if(recv > 0)
        {
            masterin.advance(recv);
            processmasterinput();
        }
        else disconnectmaster();
    }
    
    void updatemasterserver()
    {
        extern int serverport;
        if(!masterconnected && lastconnectmaster && totalmillis-lastconnectmaster <= 5*60*1000) return;
        if(mastername[0] && allowupdatemaster) requestmasterf("regserv %d\n", serverport);
        lastupdatemaster = totalmillis ? totalmillis : 1;
    }
};

vector<msinfo> mss;     // masterservers
int currmss = -1;

void addms()
{
    mss.add();
    currmss = mss.length()-1;
    mss[currmss].masternum = currmss;
}
COMMAND(addms, "");

void clearmss()
{
    server::masterdisconnected(-1);
    mss.shrink(0);
    currmss = -1;
}
COMMAND(clearmss, "");

VARFN(updatemaster, allowupdatemaster, 0, 1, 1,
{
    if(!mss.inrange(currmss)) addms();
    mss[currmss].allowupdatemaster = allowupdatemaster;
});
SVARF(mastername, server::defaultmaster(),
{
    if(!mss.inrange(currmss)) addms();
    mss[currmss].disconnectmaster();
    copystring(mss[currmss].mastername, mastername);
});
VARF(masterport, 1, server::masterport(), 0xFFFF,
{
    if(!mss.inrange(currmss)) addms();
    mss[currmss].disconnectmaster();
    mss[currmss].masterport = masterport;
});
SVARF(masterauth, "",
{
    if(!mss.inrange(currmss)) addms();
    copystring(mss[currmss].masterauth, masterauth);
});

bool requestmasterf(int m, const char *fmt, ...)
{
    if(mss.inrange(m))
    {
        defvformatstring(req, fmt, fmt);
        return mss[m].requestmaster(req);
    }
    else return false;
}

ENetSocket connectmaster(int m, bool wait)
{
    return mss.inrange(m) ? mss[m].connectmaster(wait) : ENET_SOCKET_NULL;
}

void updatemasterserver()
{
    loopv(mss) mss[i].updatemasterserver();
}

void flushmasteroutput()
{
    loopv(mss) mss[i].flushmasteroutput();
}

int findauthmaster(const char *desc, int old)
{
    for(int m = old >= 0 ? old + 1 : 0; m < mss.length(); m++) if(!strcmp(desc, mss[m].masterauth)) return m;
    return -1;
}

const char *getmastername(int m)
{
    return mss.inrange(m) ? mss[m].mastername : "";
}

const char *getmasterauth(int m)
{
    return mss.inrange(m) ? mss[m].masterauth : "";
}

int nummss() { return mss.length(); }

#endif