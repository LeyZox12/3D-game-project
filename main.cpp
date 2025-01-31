#include <iostream>
#include <algorithm>
#include <SFML/Graphics.hpp>
#include <time.h>
#include <vector>
#include "../../class/Ray.h"
#include "../../class/rayCastingDDA.h"
#include <sstream>
#include "../../class/pathfinder.h"

using namespace sf;
using namespace std;

class Enemy
{
    public:
        vec2 pos;
        int mapLen;
        int time;
        int speed = 100;
        int mapSize;
        int actionIndex = 0;
        bool isTargetSet = false;
        vec2 offset;
        vec2 currentCase = vec2(0, 0);
        pathfinder path;
        vec2 acc;
        vector<vec2> actions;
        Enemy()
        {

        }
        Enemy(vec2 pos)
        {
            this->pos = pos;
        }
        string rectCollision(RectangleShape r1, RectangleShape r2)
        {
            vec2 r1pos = r1.getPosition();
            vec2 r2pos = r2.getPosition();
            vec2 r1size = r1.getSize();
            vec2 r2size = r2.getSize();
            bool collision = (  r1pos.x + r1size.x >= r2pos.x &&
                                r1pos.x <= r2pos.x + r2size.x &&
                                r1pos.y + r1size.y >= r2pos.y &&
                                r1pos.y <= r2pos.y + r2size.y);
            if(!collision)
                return "none";
            else
            {
                vector<float> distances = {
                                            abs(r2pos.y - (r1pos.y + r1size.y)),
                                            abs(r2pos.y + r2size.y - (r1pos.y - r1size.y)),
                                            abs(r2pos.x - (r1pos.x + r1size.x)),
                                            abs(r2pos.x + r2size.x - (r1pos.x - r1size.x))
                                            };
                auto it = min_element(distances.begin(), distances.end());
                int index = distance(distances.begin(), it);
                switch(index)
                {
                    case(0):
                        return "up";
                        break;
                    case(1):
                        return "down";
                        break;
                    case(2):
                        return "left";
                        break;
                    case(3):
                        return "right";
                        break;
                }
            }
        }
        void setTarget(vector<vector<int>> mat, vec2 target)
        {
            if(!mat[target.y][target.x] == 1)
            {
                mat[target.y][target.x] = 3;

                currentCase = vec2(floor((pos.x) / mat.size()), floor((pos.y) / mat.size()));
                actionIndex = 0;
                actions =  path.getClosest(mat, currentCase, target);
                isTargetSet = true;
            }
        }
        void update(double dt)
        {
            if(isTargetSet)
            {
                RectangleShape enemyRect(vec2(10, 10));
                enemyRect.setPosition(pos);
                RectangleShape currentCell(vec2(mapSize, mapSize));
                if(actionIndex == actions.size() - 1)
                    isTargetSet = false;
                else if(clock() - time >= 500)
                {
                    time = clock();
                    currentCase += actions[actionIndex];
                    actionIndex++;

                }
                pos += vec2((currentCase.x * mapSize + mapSize / 2 - pos.x ) / 20, (currentCase.y * mapSize + mapSize / 2 - pos.y) / 20 );
            }
        }
        void display(RenderWindow& window, vec2 offset)
        {
            RectangleShape enemyRect(vec2(10, 10));
            enemyRect.setOrigin(5, 5);
            enemyRect.setPosition(pos + offset);
            enemyRect.setFillColor(Color::Red);
            window.draw(enemyRect);
        }
};
class objManager
{
    public:
        void addObject(vec2 pos, Texture texture, int yPos)
        {
            object newObj;
            newObj.setTexture(texture);
            newObj.setPosition(pos);
            newObj.setYPos(yPos);
            objects.push_back(newObj);
        }
        void drawObjects(RenderWindow& window, float dist, vec2 playerPos, float playerDir, int fov, int maxSize)
        {
            for(int i = 0; i < objects.size(); i++)
            {
                vec2 diff = vec2(objects[i].getPosition() - playerPos);
                float playerToObjDist = sqrt(diff.x * diff.x + diff.y * diff.y) * 2;
                if(dist > playerToObjDist && !objects[i].getHasBeenDrawn())
                {
                    objects[i].draw(window, playerPos, playerDir, fov, 400);
                }
            }
            for(int i = 0; i < objects.size(); i++)
                objects[i].setBeenDrawn(false);
        }
    private:
        class object
        {
            public:
                object()
                {

                }
                void setYPos(float yPos)
                {
                    this -> yPos = yPos;
                }
                int getYPos()
                {
                    return yPos;
                }
                void setBeenDrawn(bool state)
                {
                    hasBeenDrawn = state;
                }
                void setTexture(Texture texture)
                {
                    texture = texture;
                }
                Texture getTexture()
                {
                    return texture;
                }
                bool getHasBeenDrawn()
                {
                    return hasBeenDrawn;
                }
                vec2 setPosition(vec2 pos)
                {
                    this -> pos = pos;
                }
                vec2 getPosition()
                {
                    return pos;
                }
                void draw(RenderWindow& window, vec2 playerPos, float playerDir, int fov, int maxSize)
                {
                    double pi = 3.1415926;
                    double deg = 180 / pi;
                    double rad = pi / 180;
                    RectangleShape sprite;
                    vec2 oVec = vec2(pos.x - playerPos.x,
                                     pos.y - playerPos.y);
                    float eAngle = atan2(oVec.y, oVec.x) * deg;
                    float dist = sqrt(oVec.x * oVec.x + oVec.y * oVec.y);
                    float endAngle = atan2(cos(playerDir), sin(playerDir)) * deg;
                    float startAngle = atan2(cos(playerDir + fov * rad), sin(playerDir + fov * rad)) * deg;
                    float diff = endAngle - startAngle;
                    if(diff < 0)
                    {
                        endAngle += 360;
                        if(eAngle < 0)
                            eAngle += 360;
                    }
                    float percent = (eAngle - startAngle) / (endAngle - startAngle);
                    dist *= 2;
                    float rSize = dist * 2 < 0 ? 0 : (dist * 2 > maxSize ? maxSize : dist * 2);
                    vec2 endSize = vec2(maxSize - rSize, maxSize - rSize);
                    vec2 endPos = vec2(percent * window.getSize().x, yPos);
                    //centers the sprite
                    sprite.move(vec2(rSize / 2, rSize / 2));
                    float col = 255 / (dist / 100);
                    col =  col > 255 ? 255 : (col < 0 ? 0 : col);
                    sprite.setFillColor(Color(col, col, col));
                    if(eAngle > startAngle && eAngle < endAngle)
                    {
                        sprite.setSize(endSize);
                        sprite.setPosition(endPos);
                        sprite.setTexture(&texture);
                        window.draw(sprite);
                        setBeenDrawn(true);
                    }
                }
            private:
                Texture texture;
                bool hasBeenDrawn;
                vec2 pos;
                float yPos;
        };
        vector<object> objects;
};

RenderWindow window(VideoMode(900,900), "Raycaster");
Event e;
Font font;
Text fpsText;
int fps();
void start();
long double dt;
long double x;
const double rad = 0.0174533;
const double deg = 57.2958;
const int fov = 45;
int raycount;
float sens = 0.005;
typedef Vector2f vec2;
int mapSize = 20;
double dir = 0;
float playerAngle;
int oldAngle;
bool isDDA = true;
bool showMinimap = true;
RectangleShape playerRect(vec2(10, 10));
vec2 position;
vec2 vel = vec2(0, 0);
string toString(int n);
Keyboard::Key heldKey;
Texture enemyTexture;
Texture floorTexture;
Texture slotTexture;
vector<vector<int>> mapArray = {
{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
{1,0,0,1,0,0,0,0,1,1,0,0,0,0,0,1},
{1,0,0,1,0,0,0,0,0,1,1,0,0,0,0,1},
{1,0,0,1,1,1,0,0,0,0,1,0,0,0,0,1},
{1,0,0,0,0,1,0,0,0,0,1,0,0,1,0,1},
{1,0,0,0,0,0,0,0,0,0,1,0,0,1,0,1},
{1,0,0,1,1,0,0,0,0,0,0,0,0,1,1,1},
{1,0,0,0,1,1,0,0,0,0,0,0,0,0,1,1},
{1,0,0,0,0,0,0,0,1,0,1,1,0,1,1,1},
{1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1},
{1,0,0,0,1,0,0,0,0,1,0,1,1,1,0,1},
{1,0,0,0,1,0,0,0,1,0,0,1,1,0,0,1},
{1,0,1,1,1,0,0,0,1,0,0,1,0,0,1,1},
{1,0,0,0,1,0,0,0,0,0,0,1,0,0,0,1},
{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

vec2 offset = vec2(900 - mapArray.size() * mapSize, 0);
Enemy e1(vec2(40, 60));
objManager obj;
int speed=4000;
float precision = 1;
float dirAngle;
void drawMiniMap(vector<vector<int>> mapA, vec2 offset, int mapSize);
void drawMap(vector<float> distances, vector<int> sides, vector<float> percents, RenderWindow& window);
void postProcessing(vector<int> arr, int treshold);
float clamp(float val, float minval, float maxval);
void start()
{
    floorTexture.loadFromFile("res/floor.png");
    slotTexture.loadFromFile("res/slot_machine.png");
    enemyTexture.loadFromFile("res/enemy.png");
    raycount = 1;
    int randY;
    int randX;
    do
    {
        randY = rand()%mapArray.size();
        randX = rand()%mapArray[randY].size();
        position = vec2(randX*mapSize+mapSize/2 +offset.x, randY*mapSize+mapSize/2+offset.y);
    }while(mapArray[randY][randX] != 0);
    e1.mapSize = 20;
    e1.offset = offset;
    obj.addObject(e1.pos, enemyTexture, window.getSize().y / 2);

    //e1.setTarget(mapArray, position);


    window.setMouseCursorGrabbed(true);
    window.setMouseCursorVisible(false);
    window.setKeyRepeatEnabled(false);
    if(!font.loadFromFile("res/font.otf"))
        std::cout << "error: Couldn't load font.otf \n";
    fpsText.setPosition(0,0);
    fpsText.setFont(font);
    fpsText.setColor(Color::White);
}
string rectCollision(RectangleShape r1, RectangleShape r2);
int main()
{
    start();
    while(window.isOpen())
    {
        window.setTitle("Raycaster FPS:" + toString(fps()));
        while(window.pollEvent(e))
        {
            if(e.type == Event::Closed)
                window.close();

            raycount+= e.mouseWheelScroll.delta;
            if(e.type == Event::KeyPressed)
            {
                switch(e.key.code)
                {
                    case(Keyboard::Z):
                        heldKey = Keyboard::Z;
                        break;
                    case(Keyboard::Q):
                        heldKey = Keyboard::Q;
                        break;
                    case(Keyboard::S):
                        heldKey = Keyboard::S;
                        break;
                    case(Keyboard::D):
                        heldKey = Keyboard::D;
                        break;
                    case(Keyboard::Escape):
                        window.setMouseCursorGrabbed(false);
                        window.setMouseCursorVisible(true);
                        break;
                    case(Keyboard::Space):
                        isDDA = !isDDA;
                        break;
                    case(Keyboard::Tab):
                        showMinimap = !showMinimap;
                        break;
                    case(Keyboard::Up):
                        heldKey = Keyboard::Up;
                        break;
                    case(Keyboard::Down):
                        heldKey = Keyboard::Down;
                        break;
                    case(Keyboard::Add):
                        precision+=0.1;
                        break;
                    case(Keyboard::Subtract):
                        if(precision>0.1)
                            precision-=0.1;
                        break;
                }
            }
            if(e.type == Event::KeyReleased && e.key.code == heldKey)
                heldKey = Keyboard::Unknown;
            if(e.type == Event::MouseMoved)
            {
                int delta =oldAngle-e.mouseMove.x;
                dir+= delta;
                oldAngle = e.mouseMove.x;
                if(Mouse::getPosition(window).x == 0 && delta > 0)
                {
                    Mouse::setPosition(Vector2i(window.getSize().x,Mouse::getPosition(window).y),window);
                    delta = 0;
                    oldAngle = Mouse::getPosition(window).x;
                }
                if(Mouse::getPosition(window).x == window.getSize().x-1 && delta < 0)
                {
                    Mouse::setPosition(Vector2i(0,Mouse::getPosition(window).y),window);
                    delta = 0;
                    oldAngle = Mouse::getPosition(window).x;
                }
            }
        }

        if (fps() !=0)
            dt=(long double)1/fps();
        window.clear(Color::Black);
        int toAngle = (180-fov )/2;

        switch(heldKey)
        {
        case(Keyboard::Z):
            vel.x += sin(dir *sens + fov / 2 * rad) * speed * dt;
            vel.y += cos(dir * sens + fov / 2 *rad) * speed* dt;
            break;
        case(Keyboard::Q):
            vel.x += sin(dir * sens+ (180 - toAngle) * rad) * speed * dt;
            vel.y += cos(dir * sens+ (180 - toAngle) * rad) * speed * dt;
            break;
        case(Keyboard::S):
            vel.x += sin(dir * sens+ (180 + fov / 2)*rad) *speed * dt;
            vel.y += cos(dir * sens+ (180 + fov / 2)*rad) *speed * dt;
            break;
        case(Keyboard::D):
            vel.x += sin(dir*sens - toAngle*rad)* speed * dt;
            vel.y += cos(dir*sens - toAngle*rad)* speed * dt;
            break;

        }
        /*
        static int time = clock();
        if(!e1.isTargetSet)
        {
            e1.setTarget(mapArray, position);
            time = clock();
        }
        */
        window.draw(fpsText);
        static VertexArray line(LinesStrip, 2);
        vector<Ray> rays;
        vector<float> distances;
        vector<VertexArray> lines;
        vector<float> directions;
        vector<int> sides;
        vector<float> percents;
        for(float i =0; i<raycount; i+=(float)fov/raycount)
        {
            line[0].position = position;
            line[0].color = Color::Yellow;
            line[1].color = Color::Yellow;
            float angle = dir*sens+(((float)fov/raycount * i)*rad);
            float correctedDist = 0;
            vec2 dirVec = vec2(sin(angle)*20,
                               cos(angle)*20);
            playerAngle = atan2(dirVec.y, dirVec.x);
            if(isDDA)
            {
                rayCastingDDA ray;
                ray.setMapSize(20);
                vec2 fixedPos = vec2((position.x - offset.x),
                                     (position.y - offset.y));
                ray.cast(fixedPos, vec2(dirVec.x, dirVec.y), mapSize, 20, mapArray);
                correctedDist = ray.getDist();
                percents.push_back(ray.getPercent());
                sides.push_back(ray.getSide());
                lines.push_back(line);
                line[1].position = vec2(position + dirVec * correctedDist);
                distances.push_back(correctedDist * 50);
            }
            else
            {
                Ray ray(line[0].position, dirVec, 1000);
                ray.Cast(mapArray, mapSize, offset ,precision);
                line[1].position = vec2(ray.currentPoint.x,ray.currentPoint.y);
                correctedDist = ray.getDist() - (cos(dir*sens+(((float)fov/raycount * i)*rad)));
                sides.push_back(ray.side);
                lines.push_back(line);
                distances.push_back(correctedDist);
            }
        }
        vec2 fixedPos = vec2(floor((position.x -  offset.x) / mapArray.size()),
                             floor((position.y -  offset.y) / mapArray.size()));
        static vec2 oldPos = fixedPos;

        if(fixedPos != oldPos)
        {
            oldPos = fixedPos;
            //e1.setTarget(mapArray, fixedPos);
            cout << e1.actions.size() << endl << endl;
        }
        //e1.update(dt);
        playerRect.setPosition(position);
        playerRect.setOrigin(5, 5);
        for(int i = 0; i < mapArray.size(); i++)
        {
            for(int j = 0; j < mapArray[i].size(); j++)
            {
                if(mapArray[i][j] == 1)
                {
                    RectangleShape r2(vec2(mapSize, mapSize));
                    r2.setPosition(j * mapSize + offset.x, i * mapSize + offset.y);
                    string result = rectCollision(playerRect, r2);
                    float halfSize = playerRect.getSize().x / 2;
                    if(result != "none")
                        if(result == "up")
                            vel.y = clamp(vel.y, -9999, 0);
                        else if(result == "down")
                            vel.y = clamp(vel.y, 0, 9999);
                        else if(result == "left")
                            vel.x = clamp(vel.x, -9999, 0);
                        else if(result == "right")
                            vel.x = clamp(vel.x, 0, 9999);
                }
            }
        }
        position += vec2(vel.x * dt, vel.y * dt);
        vel = vec2(vel.x * 0.9, vel.y * 0.9);
        drawMap(distances, sides, percents, window);
        if(showMinimap)
        {
            drawMiniMap(mapArray, offset, 20);
            for(auto& l: lines)
                window.draw(l);
            e1.display(window, offset);
        }

        //drawObject(window, slotTexture, vec2(3 * mapSize, mapSize) + offset, position, dir * sens, fov, window.getSize().y - 200, 800);
        window.display();
    }
    return 0;
}

int fps()
{
    static int t1 = clock();
    static int fps= 0;
    static int lastFps;

    if(t1 - clock() <=-1000)
    {
        lastFps = fps;
        fps = 0;
        t1 = clock();
    }
    else
    {
        fps++;
    }
    return lastFps;
}

void drawMap(vector<float> distances, vector<int> sides, vector<float> percents, RenderWindow& window)
{
    double ratioAngle = distances.size() / fov;
    Texture wallTexture;
    wallTexture.loadFromFile("res/texture.png");
    Vector2u textureSize = wallTexture.getSize();
    wallTexture.setRepeated(true);
    RectangleShape wall;
    wall.setPosition(window.getSize().x, 0);
    wall.setTexture(&wallTexture);
    vector<pair<float, int>> distancesSorted;
    float rectSizeRatio = window.getSize().y / distances.size();
    RectangleShape floorRect;
    floorRect.setPosition(0, window.getSize().y - rectSizeRatio);
    for(int i = 0; i < distances.size(); i++)
    {
        floorRect.setSize(vec2(window.getSize().x, rectSizeRatio));
        float percent = i * rectSizeRatio / window.getSize().y;
        float col = 255 - 255 * percent * 2;
        floorRect.setTexture(&floorTexture);
        floorTexture.setRepeated(true);
        floorRect.setTextureRect({0, percent * floorTexture.getSize().y, floorTexture.getSize().x, floorTexture.getSize().y});
        floorRect.setFillColor(Color(col, col, col));
        window.draw(floorRect);
        floorRect.move(0, -rectSizeRatio);
        distancesSorted.push_back(make_pair(distances[i], i));
    }
    sort(distancesSorted.begin(), distancesSorted.end());
    reverse(distancesSorted.begin(), distancesSorted.end());

    for(auto& d : distancesSorted)
    {
        int index = d.second;
        wall.setPosition(window.getSize().x - index * window.getSize().x / distances.size(), 0);
        float dist = distances[index];
        wall.setSize(vec2(window.getSize().x / distances.size() + 1, clamp(window.getSize().y / (dist / 50), 0, window.getSize().y)));
        wall.setPosition(wall.getPosition().x, (window.getSize().y - wall.getSize().y) / 2);
        float shadowPower= 0.5;
        float col = 255 / (dist / 100);
        col = clamp(col, 0, 255);
        wall.setFillColor(Color(col, col, col,255));
        switch(sides[index])
        {
        /*case(0):
            wall.setFillColor(Color(255-d*shadowPower,0,0));
            break;
        case(1):
            wall.setFillColor(Color(0,255-d*shadowPower,0));
            break;
        case(2):
            wall.setFillColor(Color(0,0,255-d*shadowPower));
            break;
        case(3):
            wall.setFillColor(Color(255-d*shadowPower,0,255-d*shadowPower));
            break;
        default:
            break;*/
        }
        if(isDDA)
            wall.setTextureRect({fmod(percents[index] * textureSize.x, textureSize.x), 0, wall.getSize().x / textureSize.x, textureSize.y});
        Color color = wall.getFillColor();
        color.r = clamp(color.r, 0, 255);
        color.g = clamp(color.g, 0, 255);
        color.b = clamp(color.b, 0, 255);
        wall.setFillColor(color);
        obj.drawObjects(window, dist, position - offset, dir * sens, fov, 400);
        window.draw(wall);
        RectangleShape ceil(vec2(wall.getSize().x, wall.getPosition().y));
        ceil.setPosition(wall.getPosition().x, 0);
        ceil.setFillColor(Color::Black);
        window.draw(ceil);
        wall.setFillColor(Color(0,0,0));
    }
    window.draw(playerRect);
}
void drawMiniMap(vector<vector<int>> mapA, vec2 offset, int mapSize)
{
    for(int y =0; y< mapA.size(); y++)
    {
        for(int x = 0; x<mapA[y].size(); x++)
        {
            RectangleShape wall(vec2(mapSize,mapSize));
            if(mapA[y][x] != 0)
            {
                wall.setFillColor(Color::White);
                wall.setPosition(vec2(x*mapSize+offset.x ,y*mapSize+offset.y));
                window.draw(wall);
            }
        }
    }

}

string toString(int n)
{
    stringstream ss;
    ss << n;
    string buffer;
    ss>>buffer;
    return buffer;
}

void postProcessing(vector<int> arr, int treshold)
{
    int currval = arr[0];
    int index = 0;
    for(auto&  a: arr)
    {
        for(int i = index ; i<index+treshold; i++)
        {
            if(a != currval)
            {
                a=currval;
                break;
            }
            else
                currval = a;
            index++;
        }
    }
}
string rectCollision(RectangleShape r1, RectangleShape r2)
{
    vec2 r1pos = r1.getPosition();
    vec2 r2pos = r2.getPosition();
    vec2 r1size = r1.getSize();
    vec2 r2size = r2.getSize();

    bool collision = (  r1pos.x + r1size.x >= r2pos.x &&
                        r1pos.x <= r2pos.x + r2size.x &&
                        r1pos.y + r1size.y >= r2pos.y &&
                        r1pos.y <= r2pos.y + r2size.y);
    if(!collision)
        return "none";
    else
    {
        vector<float> distances = {
                                    abs(r2pos.y - (r1pos.y + r1size.y)),
                                    abs(r2pos.y + r2size.y - (r1pos.y - r1size.y)),
                                    abs(r2pos.x - (r1pos.x + r1size.x)),
                                    abs(r2pos.x + r2size.x - (r1pos.x - r1size.x))
                                    };
        auto it = min_element(distances.begin(), distances.end());
        int index = distance(distances.begin(), it);
        switch(index)
        {
            case(0):
                return "up";
                break;
            case(1):
                return "down";
                break;
            case(2):
                return "left";
                break;
            case(3):
                return "right";
                break;
        }
    }
}
float clamp(float val, float minval, float maxval)
{
    if(val < minval)
        return minval;
    if(val > maxval)
        return maxval;
    return val;
}
