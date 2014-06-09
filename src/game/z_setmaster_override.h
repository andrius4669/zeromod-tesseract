#ifndef Z_SETMASTER_OVERRIDE
#define Z_SETMASTER_OVERRIDE

#include "z_triggers.h"

VAR(defaultmastermode, -1, 0, 3);

static void z_trigger_defaultmastermode(int type)
{
    mastermode = defaultmastermode;
    packetbuf p(MAXTRANS, ENET_PACKET_FLAG_RELIABLE);
    putint(p, N_MASTERMODE);
    putint(p, mastermode);
    sendpacket(-1, 1, p.finalize());
}
Z_TRIGGER(z_trigger_defaultmastermode, Z_TRIGGER_STARTUP);

bool setmaster(clientinfo *ci, bool val, const char *pass = "", const char *authname = NULL, const char *authdesc = NULL, int authpriv = PRIV_MASTER, bool force = false, bool trial = false)
{
    if(authname && !val) return false;
    const char *name = "";
    const int opriv = ci->privilege;
    if(val)
    {
        bool haspass = adminpass[0] && checkpassword(ci, adminpass, pass);
        int wantpriv = ci->local || haspass ? PRIV_ADMIN : authpriv;
        if(ci->privilege)
        {
            if(wantpriv <= ci->privilege) return true;
        }
        else if(wantpriv <= PRIV_MASTER && !force)
        {
            if(ci->state.state==CS_SPECTATOR)
            {
                sendf(ci->clientnum, 1, "ris", N_SERVMSG, "Spectators may not claim master.");
                return false;
            }
            if(!authname && !(mastermask&MM_AUTOAPPROVE) && !ci->privilege && !ci->local)
            {
                sendf(ci->clientnum, 1, "ris", N_SERVMSG, "This server requires you to use the \"/auth\" command to claim master.");
                return false;
            }
            loopv(clients) if(ci!=clients[i] && clients[i]->privilege)
            {
                sendf(ci->clientnum, 1, "ris", N_SERVMSG, "Master is already claimed.");
                return false;
            }
        }
        if(trial) return true;
        ci->privilege = wantpriv;
        name = privname(ci->privilege);
    }
    else
    {
        if(!ci->privilege) return false;
        if(trial) return true;
        name = privname(ci->privilege);
        revokemaster(ci);
    }
    bool hasmaster = false;
    loopv(clients) if(clients[i]->local || clients[i]->privilege >= PRIV_MASTER) hasmaster = true;
    bool mmchange = false;
    if(!hasmaster)
    {
        if(mastermode != defaultmastermode) { mastermode = defaultmastermode; mmchange = true; }
        allowedips.shrink(0);
        z_exectrigger(Z_TRIGGER_NOMASTER);
    }
    string msg;
    if(val && authname)
    {
        if(authdesc && authdesc[0]) formatstring(msg, "%s claimed %s as '\fs\f5%s\fr' [\fs\f0%s\fr]", colorname(ci), name, authname, authdesc);
        else formatstring(msg, "%s claimed %s as '\fs\f5%s\fr'", colorname(ci), name, authname);
    }
    else formatstring(msg, "%s %s %s", colorname(ci), val ? "claimed" : "relinquished", name);
    
    if(val && authname)
    {
        if(authdesc) logoutf("master: %s (%d) claimed %s as '%s' [%s]", ci->name, ci->clientnum, name, authname, authdesc);
        else logoutf("master: %s (%d) claimed %s as '%s'", ci->name, ci->clientnum, name, authname);
    }
    else logoutf("master: %s (%d) %s %s", ci->name, ci->clientnum, val ? "claimed" : "relinquished", name);
    
    loopv(clients)
    {
        clientinfo *cj = clients[i];
        if(!cj->state.aitype!=AI_NONE) continue;
        // 1 case: client can see that ci claimed
        // 2 case: client could see old privilege, and ci relinquished
        // action: send out message and new privileges
        if(val ? ci->canseemypriv(cj) : ci->canseemypriv(cj, opriv))
        {
            packetbuf p(MAXTRANS, ENET_PACKET_FLAG_RELIABLE);
            putint(p, N_SERVMSG);
            sendstring(msg, p);
            putint(p, N_CURRENTMASTER);
            putint(p, mastermode);
            loopvj(clients) if(clients[j]->privilege >= PRIV_MASTER && clients[j]->canseemypriv(cj))
            {
                putint(p, clients[i]->clientnum);
                putint(p, clients[i]->privilege);
            }
            putint(p, -1);
            sendpacket(cj->clientnum, 1, p.finalize());
        }
        // 3 case: client couldn't see old privilege, but relinquish changed mastermode
        // action: send out mastermode only
        else if(!val && !ci->canseemypriv(cj, opriv) && mmchange)
        {
            packetbuf p(MAXTRANS, ENET_PACKET_FLAG_RELIABLE);
            putint(p, N_MASTERMODE);
            putint(p, mastermode);
            sendpacket(cj->clientnum, 1, p.finalize());
        }
        // 4 case: client could see old privilege, but can not see new privilege after claim
        // action: send fake relinquish message and new privileges
        else if(val && ci->canseemypriv(cj, opriv) && !ci->canseemypriv(cj))
        {
            packetbuf p(MAXTRANS, ENET_PACKET_FLAG_RELIABLE);
            putint(p, N_SERVMSG);
            sendstring(tempformatstring("%s %s %s", colorname(ci), "relinquished", privname(opriv)), p);
            putint(p, N_CURRENTMASTER);
            putint(p, mastermode);
            loopvj(clients) if(clients[j]->privilege >= PRIV_MASTER && clients[j]->canseemypriv(cj))
            {
                putint(p, clients[i]->clientnum);
                putint(p, clients[i]->privilege);
            }
            putint(p, -1);
            sendpacket(cj->clientnum, 1, p.finalize());
        }
    }
    
    checkpausegame();
    return true;
}


#endif // Z_SETMASTER_OVERRIDE