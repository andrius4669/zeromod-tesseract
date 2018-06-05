#ifdef Z_PROTECTTEAMSCORES_H
#error "already z_protectteamscores.h"
#endif
#define Z_PROTECTTEAMSCORES_H

// 0 - allow all
// 1 - don't allow players to take points they didn't earn
VAR(protectteamscores, 0, 0, 1);

static int z_calcteamscore(teaminfo *ti, int team, int fragval)
{
    return (ti[team-1].frags += fragval);
}

bool z_acceptfragval(clientinfo *ci, int fragval)
{
    switch(protectteamscores)
    {
        case 0: default: return true;
        case 1:
        {
            int teamscore = z_calcteamscore(ci->state.teaminfos, ci->team, fragval);
            return fragval > 0 ? (teamscore > 0) : (teamscore >= 0);
        }
    }
}
