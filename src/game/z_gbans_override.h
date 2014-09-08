#ifndef Z_GBANS_OVERRIDE_H
#define Z_GBANS_OVERRIDE_H

struct gbaninfo: ipmask
{
    int master;
};
vector<gbaninfo> gbans;

static void cleargbans(int m = -1)
{
    if(m < 0) loopvrev(gbans) { if(gbans[i].master >= 0) gbans.removeunordered(i); }
    else loopvrev(gbans) if(gbans[i].master == m) gbans.removeunordered(i);
}

static bool checkgban(uint ip)
{
    loopv(gbans) if(gbans[i].check(ip)) return true;
    return false;
}

static void addgban(int m, const char *name, clientinfo *actor = NULL)
{
    gbaninfo &ban = gbans.add();
    ban.parse(name);
    ban.master = m;

    loopvrev(clients)
    {
        clientinfo *ci = clients[i];
        if(ci->state.aitype != AI_NONE || ci->local || ci->privilege >= PRIV_ADMIN) continue;
        if(actor && ((ci->privilege > actor->privilege && !actor->local) || ci->clientnum == actor->clientnum)) continue;
        if(checkgban(getclientip(ci->clientnum))) disconnect_client(ci->clientnum, DISC_IPBAN);
    }
}

void pban(const char *name) { addgban(-1, name); }
COMMAND(pban, "s");
void clearpbans() { loopvrev(gbans) if(gbans[i].master < 0) gbans.removeunordered(i); }
COMMAND(clearpbans, "");

#endif // Z_GBANS_OVERRIDE_H
