// Extension lib defines
#define EXTENSION_NAME gog_galaxy
#define LIB_NAME "gog_galaxy"
#define MODULE_NAME LIB_NAME

// Defold SDK
#ifndef DLIB_LOG_DOMAIN
#define DLIB_LOG_DOMAIN LIB_NAME
#endif
#include <dmsdk/sdk.h>

#if defined(DM_PLATFORM_OSX) || defined(DM_PLATFORM_WINDOWS) || defined(DM_PLATFORM_LINUX)
#define GALAXY_ENABLED

#include <galaxy/GalaxyApi.h>

static void check_error(lua_State *L) {
    const galaxy::api::IError *error = galaxy::api::GetError();
    if (!error) { return; }
    luaL_error(L, "%s", error->GetMsg());
}

static int init(lua_State *L) {
    const char *clientID = luaL_checkstring(L, 1);
    const char *clientSecret = luaL_checkstring(L, 2);

    galaxy::api::Init(galaxy::api::InitOptions(clientID, clientSecret));
    check_error(L);
    return 0;
}

static int update(lua_State *L) {
    galaxy::api::ProcessData();
    check_error(L);
    return 0;
}

static int final(lua_State *L) {
    galaxy::api::Shutdown();
    check_error(L);
    return 0;
}

static class GOGAuthListener: public galaxy::api::IAuthListener {
    public:
        dmScript::LuaCallbackInfo *luaCallback = NULL;

    private:
        void CallLuaCallback(const char *cmd) {
            if (!luaCallback) { return; }
            lua_State* L = dmScript::GetCallbackLuaContext(luaCallback);
            DM_LUA_STACK_CHECK(L, 0);

            if (!dmScript::SetupCallback(luaCallback)) { return; }
            lua_pushstring(L, cmd);
            int ret = dmScript::PCall(L, 2, 0);
            dmScript::TeardownCallback(luaCallback);
        }

    public:
        void OnAuthSuccess() {
            CallLuaCallback("OnAuthSuccess");
        }

        void OnAuthFailure(FailureReason failureReason) {
            CallLuaCallback("OnAuthFailure");
        }

        void OnAuthLost() {
            CallLuaCallback("OnAuthLost");
        }
} g_GOGAuthListener;

static int user_sign_in_galaxy(lua_State *L) {
    bool requires_online = false;
    if (!lua_isnoneornil(L, 1)) {
        luaL_checktype(L, 1, LUA_TBOOLEAN);
        requires_online = lua_toboolean(L, 1);
    }
    if (g_GOGAuthListener.luaCallback) {
        dmScript::DestroyCallback(g_GOGAuthListener.luaCallback);
        g_GOGAuthListener.luaCallback = NULL;
    }
    if (!lua_isnoneornil(L, 2)) {
        g_GOGAuthListener.luaCallback = dmScript::CreateCallback(L, 2);
    }
    galaxy::api::User()->SignInGalaxy(requires_online, &g_GOGAuthListener);
    check_error(L);
    return 0;
}

static class GOGUserStatsAndAchievementsRetrieveListener: public galaxy::api::IUserStatsAndAchievementsRetrieveListener {
    public:
        dmScript::LuaCallbackInfo *luaCallback = NULL;

    private:
        void CallLuaCallback(const char *cmd) {
            if (!luaCallback) { return; }
            lua_State* L = dmScript::GetCallbackLuaContext(luaCallback);
            DM_LUA_STACK_CHECK(L, 0);

            if (!dmScript::SetupCallback(luaCallback)) { return; }
            lua_pushstring(L, cmd);
            int ret = dmScript::PCall(L, 2, 0);
            dmScript::TeardownCallback(luaCallback);
        }

    public:
        void OnUserStatsAndAchievementsRetrieveSuccess(galaxy::api::GalaxyID userID) {
            CallLuaCallback("OnUserStatsAndAchievementsRetrieveSuccess");
        }

        void OnUserStatsAndAchievementsRetrieveFailure(galaxy::api::GalaxyID userID, FailureReason failureReason) {
            CallLuaCallback("OnUserStatsAndAchievementsRetrieveFailure");
        }
} g_GOGUserStatsAndAchievementsRetrieveListener;

static int stats_request_user_stats_and_achievements(lua_State *L) {
    if (g_GOGUserStatsAndAchievementsRetrieveListener.luaCallback) {
        dmScript::DestroyCallback(g_GOGUserStatsAndAchievementsRetrieveListener.luaCallback);
        g_GOGUserStatsAndAchievementsRetrieveListener.luaCallback = NULL;
    }
    if (!lua_isnoneornil(L, 2)) {
        g_GOGUserStatsAndAchievementsRetrieveListener.luaCallback = dmScript::CreateCallback(L, 2);
    }
    galaxy::api::Stats()->RequestUserStatsAndAchievements(galaxy::api::GalaxyID(), &g_GOGUserStatsAndAchievementsRetrieveListener);
    check_error(L);
    return 0;
}

static int stats_store_stats_and_achievements(lua_State *L) {
    galaxy::api::Stats()->StoreStatsAndAchievements();
    check_error(L);
    return 0;
}

static int stats_set_achievement(lua_State *L) {
    galaxy::api::Stats()->SetAchievement(luaL_checkstring(L, 1));
    check_error(L);
    return 0;
}

static int stats_clear_achievement(lua_State *L) {
    galaxy::api::Stats()->ClearAchievement(luaL_checkstring(L, 1));
    check_error(L);
    return 0;
}

static const luaL_reg Module_methods[] =
{
    {"init", init},
    {"update", update},
    {"final", final},
    {"user_sign_in_galaxy", user_sign_in_galaxy},
    {"stats_request_user_stats_and_achievements", stats_request_user_stats_and_achievements},
    {"stats_store_stats_and_achievements", stats_store_stats_and_achievements},
    {"stats_set_achievement", stats_set_achievement},
    {"stats_clear_achievement", stats_clear_achievement},
    {0, 0}
};

static void LuaInit(lua_State* L)
{
    int top = lua_gettop(L);

    // Register lua names
    luaL_register(L, MODULE_NAME, Module_methods);

    lua_pop(L, 1);
    assert(top == lua_gettop(L));
}

#endif

static dmExtension::Result AppInitializeExtension(dmExtension::AppParams* params) {
    return dmExtension::RESULT_OK;
}

static dmExtension::Result InitializeExtension(dmExtension::Params* params) {
#ifdef GALAXY_ENABLED
    LuaInit(params->m_L);
#endif
    return dmExtension::RESULT_OK;
}

static dmExtension::Result AppFinalizeExtension(dmExtension::AppParams* params) {
    return dmExtension::RESULT_OK;
}

static dmExtension::Result FinalizeExtension(dmExtension::Params* params) {
    return dmExtension::RESULT_OK;
}

DM_DECLARE_EXTENSION(EXTENSION_NAME, LIB_NAME, AppInitializeExtension, AppFinalizeExtension, InitializeExtension, 0, 0, FinalizeExtension)
