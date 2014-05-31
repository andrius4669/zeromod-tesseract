#ifndef Z_GENERICSERVERCOMMANDS_H
#define Z_GENERICSERVERCOMMANDS_H 1

#include "z_servcmd.h"

static char z_privcolor(int priv)
{
    if(priv <= PRIV_NONE) return '7';
    else if(priv <= PRIV_AUTH) return '0';
    else return '6';
}

static void z_servcmd_commands(int argc, char **argv, int sender)
{
    vector<char> cbufs[PRIV_ADMIN+1];
    clientinfo *ci = getinfo(sender);
    loopv(z_servcommands)
    {
        z_servcmdinfo &c = z_servcommands[i];
        if(!c.valid() || c.hidden) continue;
        if(ci && !ci->local && ci->privilege < c.privilege) continue;
        int j = 0;
        if(c.privilege >= PRIV_ADMIN) j = PRIV_ADMIN;
        else if(c.privilege >= PRIV_AUTH) j = PRIV_AUTH;
        else if(c.privilege >= PRIV_MASTER) j = PRIV_MASTER;
        if(cbufs[j].empty()) { cbufs[j].add('\f'); cbufs[j].add(z_privcolor(j)); }
        else { cbufs[j].add(','); cbufs[j].add(' '); }
        cbufs[j].put(c.name, strlen(c.name));
    }
    sendf(sender, 1, "ris", N_SERVMSG, "\f2avaiable server commands:");
    loopi(sizeof(cbufs)/sizeof(cbufs[0]))
    {
        if(cbufs[i].empty()) continue;
        cbufs[i].add('\0');
        sendf(sender, 1, "ris", N_SERVMSG, cbufs[i].getbuf());
    }
}
SCOMMANDNA(commands, PRIV_NONE, z_servcmd_commands, 1);
SCOMMANDNAH(help, PRIV_NONE, z_servcmd_commands, 1);

static const struct z_timedivinfo { const char *name; int timediv; } z_timedivinfos[] =
{
    // month is inaccurate 
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
        if(m_ctf) formatstring(buf,
            "\f6stats: \f7%s: \f2frags: \f7%d\f2, flags: \f7%d\f2, deaths: \f7%d\f2, teamkills: \f7%d\f2, accuracy(%%): \f7%d\f2, kpd: \f7%.2f",
            colorname(ci), ci->state.frags, ci->state.flags, ci->state.deaths, ci->state.teamkills,
            ci->state.damage*100/max(ci->state.shotdamage,1), float(ci->state.frags)/max(ci->state.deaths,1));
        else formatstring(buf,
            "\f6stats: \f7%s: \f2frags: \f7%d\f2, deaths: \f7%d\f2, teamkills: \f7%d\f2, accuracy(%%): \f7%d\f2, kpd: \f7%.2f",
            colorname(ci), ci->state.frags, ci->state.deaths, ci->state.teamkills,
            ci->state.damage*100/max(ci->state.shotdamage,1), float(ci->state.frags)/max(ci->state.deaths,1));
        sendf(sender, 1, "ris", N_SERVMSG, buf);
    }
    return;
fail:
    sendf(sender, 1, "ris", N_SERVMSG, tempformatstring("unknown client: %s", argv[i]));
}
SCOMMANDN(stats, PRIV_NONE, z_servcmd_stats);

VAR(servcmd_pm_comfirmation, 0, 1, 1);

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
    if(servcmd_pm_comfirmation)
    {
        ci = getinfo(cn);
        sendf(sender, 1, "ris", N_SERVMSG, tempformatstring("\f2your private message successfully sent to %s \f5(%d)", ci->name, ci->clientnum));
    }
    return;
cnfail:
    sendf(sender, 1, "ris", N_SERVMSG, tempformatstring("unknown client: %s", argv[1]));
}
SCOMMANDNA(pm, PRIV_NONE, z_servcmd_pm, 2);

#include "z_mutes.h"

#include "z_loadmap.h"
#include "z_savemap.h"

#endif //Z_GENERICSERVERCOMMANDS_H
