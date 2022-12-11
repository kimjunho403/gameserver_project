math.randomseed(os.time())

myid = 99999;
level = math.random(10)
hp = 10
power = level *10
exp = level*5

function set_uid(x)
   myid = x;
end

function set_hp(x,player)
   hp =hp - x;
   if(hp <=0) then
       API_MonsterDie(player,exp);
   end
end

function get_exp()
   return exp;
end

function event_player_move(player)
   player_x = API_get_x(player);
   player_y = API_get_y(player);
   my_x = API_get_x(myid);
   my_y = API_get_y(myid);
   if (player_x == my_x) then
      if (player_y == my_y) then
         API_SendMessage(myid, player, "HELLO");
      end
   end
end
