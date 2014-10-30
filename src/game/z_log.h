#ifndef Z_LOG_H
#define Z_LOG_H

static inline
void z_log_kick(clientinfo *actor, const char *aname, const char *adesc, int apriv, clientinfo *victim, const char *reason)
{
    const char *kicker;
    if(!aname) kicker = tempformatstring("%s (%d)", actor->name, actor->clientnum);
    else
    {
        if(adesc && adesc[0]) kicker = tempformatstring("%s (%d) as '%s' [%s] (%s)", actor->name, actor->clientnum, aname, adesc, privname(apriv));
        else kicker = tempformatstring("%s (%d) as '%s' (%s)", actor->name, actor->clientnum, aname, privname(apriv));
    }
    if(reason && *reason) logoutf("kick: %s kicked %s (%d) because: %s", kicker, victim->name, victim->clientnum, reason);
    else logoutf("kick: %s kicked %s (%d)", kicker, victim->name, victim->clientnum);
}

static inline
void z_log_setmaster(clientinfo *master, bool val, bool pass, const char *aname, const char *adesc, const char *priv, clientinfo *by)
{
    const char *mode;
    if(by) mode = tempformatstring("%s by %s (%d)", val ? "passed" : "taken", by->name, by->clientnum); // someone else used /setmaster
    else if(aname) mode = "auth";                   // auth system
    else if(pass) mode = "password";     // password claim
    else mode = NULL;
    if(aname)
    {
        if(adesc && adesc[0]) logoutf("master: %s (%d) claimed %s as '%s' [%s] (%s)", master->name, master->clientnum, priv, aname, adesc, mode);
        else logoutf("master: %s (%d) claimed %s as '%s' (%s)", master->name, master->clientnum, priv, aname, mode);
    }
    else
    {
        if(mode) logoutf("master: %s (%d) %s %s (%s)", master->name, master->clientnum, val ? "claimed" : "relinquished", priv, mode);
        else logoutf("master: %s (%d) %s %s", master->name, master->clientnum, val ? "claimed" : "relinquished", priv);
    }
}

#endif // Z_LOG_H