#ifndef Z_GBANS_OVERRIDE_H
#define Z_GBANS_OVERRIDE_H 1

struct gbaninfo
{
    enet_uint32 ip, mask;
    int master;
};

vector<gbaninfo> gbans;

void cleargbans(int m = -1)
{
    if(m < 0) gbans.shrink(0);
    else loopvrev(gbans) if(gbans[i].master == m) gbans.removeunordered(i);
}

bool checkgban(uint ip)
{
    loopv(gbans) if((ip & gbans[i].mask) == gbans[i].ip) return true;
    return false;
}

void addgban(int m, const char *name)
{
    union { uchar b[sizeof(enet_uint32)]; enet_uint32 i; } ip, mask;
    ip.i = 0;
    mask.i = 0;
    const char *cidr = strchr(name, '/');
    loopi(4)
    {
        char *end = NULL;
        int n = strtol(name, &end, 10);
        if(!end) break;
        if(end > name) { ip.b[i] = n; mask.b[i] = 0xFF; }
        name = end;
        while(*name && *name++ != '.');
    }
    if(cidr && *++cidr)
    {
        int n = atoi(cidr);
        if(n > 0)
        {
            for(int i = n; i < 32; i++) mask.b[i/8] &= ~(1 << (7 - (i & 7)));
            ip.i &= mask.i;
        }
    }
    gbaninfo &ban = gbans.add();
    ban.ip = ip.i;
    ban.mask = mask.i;
    ban.master = m;
    
    loopvrev(clients)
    {
        clientinfo *ci = clients[i];
        if(ci->state.aitype != AI_NONE || ci->local || ci->privilege >= PRIV_ADMIN) continue;
        if(checkgban(getclientip(ci->clientnum))) disconnect_client(ci->clientnum, DISC_IPBAN);
    }
}

#endif //Z_GBANS_OVERRIDE_H
