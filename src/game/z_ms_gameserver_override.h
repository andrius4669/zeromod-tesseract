#ifndef Z_MS_GAMESERVER_OVERRIDE_H
#define Z_MS_GAMESERVER_OVERRIDE_H 1

#include "z_gbans_override.h"

clientinfo *findauth(int m, uint id)
{
    loopv(clients) if(clients[i]->authmaster == m && clients[i]->authreq == id) return clients[i];
    loopv(connects) if(connects[i]->authmaster == m && connects[i]->authreq == id) return connects[i];
    return NULL;
}

uint nextauthreq = 0;

void authfailed(clientinfo *ci)
{
    if(!ci) return;
    int om = ci->authmaster;
    ci->cleanauth(false);
    for(int m = findauthmaster(ci->authdesc, om); m >= 0; m = findauthmaster(ci->authdesc, m))
    {
        if(!ci->authreq) { if(!nextauthreq) nextauthreq = 1; ci->authreq = nextauthreq++; }
        if(requestmasterf(m, "reqauth %u %s\n", ci->authreq, ci->authname)) { ci->authmaster = m; break; }
    }
    if(ci->authmaster < 0)
    {
        ci->cleanauth();
        if(ci->connectauth) disconnect_client(ci->clientnum, ci->connectauth);
    }
}

void authfailed(int m, uint id)
{
    authfailed(findauth(m, id));
}

void authsucceeded(int m, uint id, int priv = PRIV_AUTH)
{
    clientinfo *ci = findauth(m, id);
    if(!ci) return;
    ci->cleanauth(ci->connectauth!=0);
    if(ci->connectauth) connected(ci);
    if(ci->authkickvictim >= 0)
    {
        if(setmaster(ci, true, "", ci->authname, ci->authdesc, priv, false, true))
            trykick(ci, ci->authkickvictim, ci->authkickreason, ci->authname, ci->authdesc, priv);
        ci->cleanauthkick();
    }
    else setmaster(ci, true, "", ci->authname, ci->authdesc, priv, false, false);
}

void authchallenged(int m, uint id, const char *val, const char *desc = "")
{
    clientinfo *ci = findauth(m, id);
    if(!ci) return;
    sendf(ci->clientnum, 1, "risis", N_AUTHCHAL, desc, id, val);
}

bool tryauth(clientinfo *ci, const char *user, const char *desc)
{
    ci->cleanauth();
    if(!nextauthreq) nextauthreq = 1;
    ci->authreq = nextauthreq++;
    filtertext(ci->authname, user, false, 100);
    copystring(ci->authdesc, desc);
    userinfo *u = users.access(userkey(ci->authname, ci->authdesc));
    if(u)
    {
        uint seed[3] = { ::hthash(serverauth) + detrnd(size_t(ci) + size_t(user) + size_t(desc), 0x10000), uint(totalmillis), randomMT() };
        vector<char> buf;
        ci->authchallenge = genchallenge(u->pubkey, seed, sizeof(seed), buf);
        sendf(ci->clientnum, 1, "risis", N_AUTHCHAL, desc, ci->authreq, buf.getbuf());
    }
    else
    {
        for(int m = findauthmaster(desc); m >= 0; m = findauthmaster(desc, m))
            if(requestmasterf(m, "reqauth %u %s\n", ci->authreq, ci->authname)) { ci->authmaster = m; break; }
        if(ci->authmaster < 0)
        {
            ci->cleanauth();
            if(!ci->authdesc[0]) sendf(ci->clientnum, 1, "ris", N_SERVMSG, "not connected to authentication server");
        }
    }
    if(ci->authreq) return true;
    if(ci->connectauth) disconnect_client(ci->clientnum, ci->connectauth);
    return false;
}

bool answerchallenge(clientinfo *ci, uint id, char *val, const char *desc)
{
    if(ci->authreq != id || strcmp(ci->authdesc, desc))
    {
        ci->cleanauth();
        return !ci->connectauth;
    }
    for(char *s = val; *s; s++)
    {
        if(!isxdigit(*s)) { *s = '\0'; break; }
    }
    int om = ci->authmaster;
    if(om < 0)
    {
        if(ci->authchallenge && checkchallenge(val, ci->authchallenge))
        {
            userinfo *u = users.access(userkey(ci->authname, ci->authdesc));
            if(u)
            {
                if(ci->connectauth) connected(ci);
                if(ci->authkickvictim >= 0)
                {
                    if(setmaster(ci, true, "", ci->authname, ci->authdesc, u->privilege, false, true))
                        trykick(ci, ci->authkickvictim, ci->authkickreason, ci->authname, ci->authdesc, u->privilege);
                }
                else setmaster(ci, true, "", ci->authname, ci->authdesc, u->privilege, false, false);
            }
        }
        ci->cleanauth();
    }
    else if(!requestmasterf(om, "confauth %u %s\n", id, val))
    {
        ci->cleanauth(false);
        for(int m = findauthmaster(desc, om); m >= 0; m = findauthmaster(desc, m))
        {
            if(!ci->authreq) { if(!nextauthreq) nextauthreq = 1; ci->authreq = nextauthreq++; }
            if(requestmasterf(m, "reqauth %u %s\n", ci->authreq, ci->authname)) { ci->authmaster = m; break; }
        }
        if(ci->authmaster < 0)
        {
            ci->cleanauth();
            if(!desc[0]) sendf(ci->clientnum, 1, "ris", N_SERVMSG, "not connected to authentication server");
        }
    }
    return ci->authreq || !ci->connectauth;
}

void masterconnected(int m)
{
    if(m >= 0) cleargbans(m);
}

void masterdisconnected(int m)
{
    if(m < 0) { cleargbans(-1); return; }
    loopvrev(clients)
    {
        clientinfo *ci = clients[i];
        if(ci->authmaster == m && ci->authreq) authfailed(ci);
    }
}

void processmasterinput(int m, const char *cmd, int cmdlen, const char *args)
{
    uint id;
    string val;
    if(sscanf(cmd, "failauth %u", &id) == 1)
        authfailed(m, id);
    else if(sscanf(cmd, "succauth %u", &id) == 1)
        authsucceeded(m, id);
    else if(sscanf(cmd, "chalauth %u %255s", &id, val) == 2)
        authchallenged(m, id, val, getmasterauth(m));
    else if(!strncmp(cmd, "cleargbans", cmdlen))
        cleargbans(m);
    else if(sscanf(cmd, "addgban %100s", val) == 1)
        addgban(m, val);
}

#endif //Z_MS_GAMESERVER_OVERRIDE_H
