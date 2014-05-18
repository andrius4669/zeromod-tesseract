#ifndef Z_GENERICSERVERCOMMANDS_H
#define Z_GENERICSERVERCOMMANDS_H 1

#include "z_servercommands.h"

static const struct z_timedivinfo { const char *name; int timediv; } z_timedivinfos[] =
{
    // I don't include "month" cuz its not exact
    { "week", 60*60*24*7 },
    { "day", 60*60*24 },
    { "hour", 60*60 },
    { "minute", 60 },
    { "second", 1 }
};

void z_servcmd_info(int argc, char **argv, int sender)
{
    vector<char> uptimebuf;
    uint secs = totalsecs;
    loopi(sizeof(z_timedivinfos)/sizeof(z_timedivinfos[0]))
    {
        uint t = secs / z_timedivinfos[i].timediv;
        if(!t) continue;
        secs %= z_timedivinfos[i].timediv;
        if(uptimebuf.length()) uptimebuf.add(' ');
        const char *timestr = tempformatstring("%u", t);
        uptimebuf.put(timestr, strlen(timestr));
        uptimebuf.add(' ');
        uptimebuf.put(z_timedivinfos[i].name, strlen(z_timedivinfos[i].name));
        if(t > 1) uptimebuf.add('s');
        if(!secs) break;
    }
    uptimebuf.add('\0');
    sendf(sender, 1, "ris", N_SERVMSG, tempformatstring("server uptime: %s", uptimebuf.getbuf()));
}
SCOMMANDNA(info, PRIV_NONE, z_servcmd_info, 1);

void z_servcmd_stats(int argc, char **argv, int sender)
{
    int cn;
    clientinfo *ci = NULL, *senderci = getinfo(sender);
    vector<clientinfo *> cis;
    int i;
    for(i = 1; i < argc; i++)
    {
        if(!z_parseclient(argv[i], &cn)) goto fail;
        if(cn < 0)
        {
            cis.shrink(0);
            loopvj(clients) if(!clients[j]->spy || !senderci || senderci->local || senderci->privilege>=PRIV_ADMIN) cis.add(clients[j]);
            break;
        }
        ci = getinfo(cn);
        if(!ci || !ci->connected || (ci->spy && senderci && !senderci->local && senderci->privilege<PRIV_ADMIN)) goto fail;
        if(cis.find(ci)<0) cis.add(ci);
    }

    if(cis.empty() && senderci) cis.add(senderci);

    for(i = 0; i < cis.length(); i++)
    {
        ci = cis[i];
        string buf;
        if(m_ctf) formatstring(buf, "\fs\f2stats: %s: frags: %d, flags: %d, deaths: %d, teamkills: %d, accuracy(%%): %d, kpd: %.2f\fr", colorname(ci),
            ci->state.frags, ci->state.flags, ci->state.deaths, ci->state.teamkills,
            ci->state.damage*100/max(ci->state.shotdamage,1), float(ci->state.frags)/max(ci->state.deaths,1));
        else formatstring(buf, "\fs\f2stats: %s: frags: %d, deaths: %d, teamkills: %d, accuracy(%%): %d, kpd: %.2f\fr", colorname(ci),
            ci->state.frags, ci->state.deaths, ci->state.teamkills, ci->state.damage*100/max(ci->state.shotdamage,1),
            float(ci->state.frags)/max(ci->state.deaths,1));
        sendf(sender, 1, "ris", N_SERVMSG, buf);
    }
    return;
fail:
    sendf(sender, 1, "ris", N_SERVMSG, tempformatstring("unknown client: %s", argv[i]));
}
SCOMMANDN(stats, PRIV_NONE, z_servcmd_stats);

void z_servcmd_pm(int argc, char **argv, int sender)
{
    if(argc <= 2) { sendf(sender, 1, "ris", N_SERVMSG, "please specify client and message"); return; }
    int cn;
    clientinfo *ci;
    if(!z_parseclient(argv[1], &cn)) goto cnfail;
    ci = getinfo(cn);
    if(!ci || !ci->connected) goto cnfail;
    if(ci->state.aitype!=AI_NONE) { sendf(sender, 1, "ris", N_SERVMSG, "you can not send private message to bot"); return; }
    ci = getinfo(sender);
    if(!ci) return;
    sendf(cn, 1, "ris", N_SERVMSG, tempformatstring("\f2pm: \f7%s \f5(%d)\f7: \f0%s", ci->name, ci->clientnum, argv[2]));
    return;
cnfail:
    sendf(sender, 1, "ris", N_SERVMSG, tempformatstring("unknown client: %s", argv[1]));
}
SCOMMANDNA(pm, PRIV_NONE, z_servcmd_pm, 2);

#endif //Z_GENERICSERVERCOMMANDS_H
