-- luacheck: globals gog_galaxy
local M = {}

local inited = false

local init_queue = {}
local user_stats_queue = {}

local function warncall(f, ...)
  local ok, err = pcall(f, ...)
  if not ok then
    print("ERROR: GOG Galaxy:", err)
  else
    return err
  end
end

local function clear_queue(queue)
  for k, v in pairs(queue) do
    queue[k] = nil
  end
end

function M.init()
  if gog_galaxy then
    local ok, err = pcall(function ()
      local client_id = tostring(sys.get_config("gog.client_id"))
      local client_secret = tostring(sys.get_config("gog.client_secret"))

      gog_galaxy.init(client_id, client_secret)
      inited = true
      print("GOG Galaxy: Initialised")

      gog_galaxy.user_sign_in_galaxy(false, function (_, cmd)
        if cmd == "OnAuthSuccess" then
          print("GOG Galaxy: Logged in")
          gog_galaxy.stats_request_user_stats_and_achievements(nil, function (_, cmd_, galaxy_id, err)
            if not user_stats_queue.resolved and cmd_ == "OnUserStatsAndAchievementsRetrieveSuccess" then
              user_stats_queue.resolved = true
              print("GOG Galaxy: Retrieved achievements")
              for _, item in ipairs(user_stats_queue) do
                item.f(unpack(item.args))
              end
            end
          end)
        end
      end)
    end)

    if ok then
      init_queue.resolved = true
      for _, item in ipairs(init_queue) do
        item.f(unpack(item.args))
      end
    else
      print("ERROR: GOG Galaxy:", err)
      clear_queue(init_queue)
    end
  end
end

function M.final()
  if inited then
    inited = false
    clear_queue(init_queue)
    clear_queue(user_stats_queue)
    warncall(gog_galaxy.final)
  end
end

function M.update()
  if inited then
    warncall(gog_galaxy.update)
  end
end

local function queued(queue, f)
  return function (...)
    if not gog_galaxy then return end
    if queue.resolved then
      return f(...)
    else
      queue[#queue + 1] = { f = f, args = { ... } }
    end
  end
end

M.set_achievement = queued(user_stats_queue, function (id)
  return warncall(gog_galaxy.stats_set_achievement, id)
end)

M.clear_achievement = queued(user_stats_queue, function (id)
  return warncall(gog_galaxy.stats_clear_achievement, id)
end)

M.store = queued(user_stats_queue, function ()
  return warncall(gog_galaxy.stats_store_stats_and_achievements)
end)

return M
