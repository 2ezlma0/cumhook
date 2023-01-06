#include "discord.h"

void Discord::Initialize()
{
    DiscordEventHandlers Handle;
    memset(&Handle, 0, sizeof(Handle));
    Discord_Initialize("938130305708064770", &Handle, 1, NULL);
}

void Discord::Update()
{
    //Discord Visualizer
    DiscordRichPresence discordPresence;
    memset(&discordPresence, 0, sizeof(discordPresence));
    discordPresence.state = "railing femboys in 2018 hvh";
    discordPresence.details = ":3";
    discordPresence.largeImageKey = "o1gmdboe_4x";
    discordPresence.largeImageText = "cumhook";
    discordPresence.smallImageKey = "o1gmdboe_4x";
    discordPresence.smallImageText = "cumhook";
    Discord_UpdatePresence(&discordPresence);
}