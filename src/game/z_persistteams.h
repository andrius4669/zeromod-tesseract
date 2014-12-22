#ifndef Z_PERSISTTEAMS_H
#define Z_PERSISTTEAMS_H

#include "z_servercommands.h"

int z_persistteams = 0;
VARFN(persistteams, defaultpersistteams, 0, 0, 1, { if(clients.empty()) z_persistteams = defaultpersistteams; });
static void z_persistteams_trigger(int type) { z_persistteams = defaultpersistteams!=0; }
Z_TRIGGER(z_persistteams_trigger, Z_TRIGGER_STARTUP);
Z_TRIGGER(z_persistteams_trigger, Z_TRIGGER_NOCLIENTS);

void z_servcmd_persistteams(int argc, char **argv, int sender)
{
    if(argc < 2) { sendf(sender, 1, "ris", N_SERVMSG, tempformatstring("persistent teams are %s", z_persistteams ? "enabled" : "disabled")); return; }
    z_persistteams = atoi(argv[1]) > 0 ? 1 : 0;
    sendservmsgf("persistent teams %s", z_persistteams ? "enabled" : "disabled");
}
SCOMMANDAH(persistteams, PRIV_MASTER, z_servcmd_persistteams, 1);
SCOMMANDA(persist, PRIV_MASTER, z_servcmd_persistteams, 1);

void z_autoteam()
{
    if(z_persistteams == 1)
    {
        vector<clientinfo *> team[MAXTEAMS];
        float teamrank[MAXTEAMS] = {0};
        int remaining = clients.length();
        loopv(clients) if(validteam(clients[i]->team))
        {
            clientinfo *ci = clients[i];
            float rank = ci->state.state!=CS_SPECTATOR ? ci->state.effectiveness/max(ci->state.timeplayed, 1) : -1;
            if(smode && smode->hidefrags()) rank = 1;
            ci->state.timeplayed = -1;
            team[ci->team-1].add(ci);
            if(rank>0) teamrank[ci->team-1] += rank;
            remaining--;
        }
        for(int round = 0; remaining>=0; round++)
        {
            int first = round&1, second = (round+1)&1, selected = 0;
            while(teamrank[first] <= teamrank[second])
            {
                float rank;
                clientinfo *ci = choosebestclient(rank);
                if(!ci) break;
                if(smode && smode->hidefrags()) rank = 1;
                else if(selected && rank<=0) break;
                ci->state.timeplayed = -1;
                team[first].add(ci);
                if(rank>0) teamrank[first] += rank;
                selected++;
                if(rank<=0) break;
            }
            if(!selected) break;
            remaining -= selected;
        }
        loopi(MAXTEAMS) loopvj(team[i])
        {
            clientinfo *ci = team[i][j];
            if(ci->team == 1+i) continue;
            ci->team = 1+i;
            sendf(-1, 1, "riiii", N_SETTEAM, ci->clientnum, ci->team, -1);
        }
    }
}

#endif // Z_PERSISTTEAMS_H