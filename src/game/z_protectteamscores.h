#ifndef Z_PROTECTTEAMSCORES_H
#define Z_PROTECTTEAMSCORES_H

VAR(protectteamscores, 0, 0, 1);

static inline int z_calcteamscore(teaminfo *ti, int team, int fragval)
{
    return (ti[team-1].frags += fragval);
}

bool z_acceptfragval(clientinfo *ci, int fragval)
{
    int teamscore = z_calcteamscore(ci->state.teaminfos, ci->team, fragval);
    switch(protectteamscores)
    {
        case 0: default: return true;
        case 1: return fragval > 0 ? (teamscore > 0) : (teamscore >= 0);
        //case 2: return fragval > 0 ? (ci->state.frags > 0) : (ci->state.frags >= 0);
    }
}

#endif // Z_PROTECTTEAMSCORES_H
