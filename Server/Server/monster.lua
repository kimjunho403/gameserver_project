math.randomseed()

name = "bluesnail"
myid = 99999;
level = math.random(10)
hp = 10* level
power = level *10
exp = level*level*2
x = math.random(2000)
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
   API_MonsterHit(myid,player,hp);
   if(hp <=0) then
       API_MonsterDie(myid,player,exp);
       hp= 10* level
   end
end

function get_exp()
   return exp;
end

function event_attack(player)
   player_x = API_get_x(player);
   player_y = API_get_y(player);
   my_x = API_get_x(myid);
   my_y = API_get_y(myid);
   if (player_x == my_x) then
      if (player_y == my_y) then
         API_attack(myid, player);
      end
   end
end
