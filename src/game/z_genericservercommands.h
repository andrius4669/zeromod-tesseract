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
SCOMMANDN(info, PRIV_NONE, z_servcmd_info);

void z_servcmd_stats(int argc, char **argv, int sender)
{
    int cn;
    clientinfo *ci = NULL;
    if(argc >= 2)
    {
        if(!z_parseclient(argv[1], &cn)) goto fail;
        if(cn < 0) goto success;
        ci = getinfo(cn);
        if(!ci) goto fail;
        goto success;
    fail:
        sendf(sender, 1, "ris", N_SERVMSG, tempformatstring("unknown client: %s", argv[1]));
        return;
    success:
        ;
    }
    else
    {
        cn = sender;
        ci = getinfo(sender);
        if(!ci) return;
    }
    if(cn >= 0) goto printinfo;
    int i;
    for(i = 0; i < clients.length(); i++)
    {
        ci = clients[i];
        goto printinfo;
continueinfo:
        ;
    }
    goto done;
printinfo:
    string buf;
    if(m_ctf) formatstring(buf, "\fs\f2stats: %s: frags: %d, flags: %d, deaths: %d, teamkills: %d, accuracy(%%): %d, kpd: %.2f\fr", colorname(ci),
        ci->state.frags, ci->state.flags, ci->state.deaths, ci->state.teamkills,
        ci->state.damage*100/max(ci->state.shotdamage,1), float(ci->state.frags)/max(ci->state.deaths, 1));
    else formatstring(buf, "\fs\f2stats: %s: frags: %d, deaths: %d, teamkills: %d, accuracy(%%): %d, kpd: %.2f\fr", colorname(ci),
        ci->state.frags, ci->state.deaths, ci->state.teamkills, ci->state.damage*100/max(ci->state.shotdamage,1),
        float(ci->state.frags)/max(ci->state.deaths, 1));
    sendf(sender, 1, "ris", N_SERVMSG, buf);
    if(cn < 0) goto continueinfo;
done:
    ;
}
SCOMMANDN(stats, PRIV_NONE, z_servcmd_stats);

#endif //Z_GENERICSERVERCOMMANDS_H
