math.randomseed()

name = "Horny Mushroom"
myid = 99999;
level = math.random(10)
hp = 10* level
power = level *5
exp = level*level*2
x = math.random(990)+1010
y = math.random(2000)

function set_uid(x)
   myid = x;
end

function get_info()
   return myid, name, level, hp, power, exp, x, y
end

function getrespawn()
   return hp,x, y
end

function set_hp(x,player)
   hp =hp - x;
  API_MonsterHit(x,myid,player,hp);
   if(hp <=0) then
       API_MonsterDie(myid,player,exp);
       hp= 10* level
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
   if (math.abs(player_x - my_x) < 5) then
      if (math.abs(player_y - my_y)< 5) then
         return 1
      end
   end
   return 0
end

function event_attack(player)
   player_x = API_get_x(player);
   player_y = API_get_y(player);
   my_x = API_get_x(myid);
   my_y = API_get_y(myid);
   if (player_x == my_x) then
      if (player_y == my_y) then
         API_attack(myid, player)
      end
   end
end
