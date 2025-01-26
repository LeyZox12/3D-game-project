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
        bool isTargetSet = false;
        vec2 offset;
        int actionsIndex;
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
            target = vec2(floor((target.x - offset.x) / mat.size()), floor((target.y - offset.y) / mat.size()));
            if(!mat[target.y][target.x] == 1)
            {
                mat[target.y][target.x] = 3;
                currentCase = vec2(floor((pos.x) / mat.size()), floor((pos.y) / mat.size()));
                actionsIndex = 0;
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
                if(actionsIndex == actions.size() - 1)
                    isTargetSet = false;
                else if(clock() - time >= 500)
                {
                    time = clock();
                    currentCase += actions[actionsIndex];
                    actionsIndex++;
                    cout << currentCase.x << endl;
                    cout << currentCase.y << endl;
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
const int fov = 90;
int raycount;
float sens = 0.005;
typedef Vector2f vec2;
int mapSize = 20;
double dir = 0;
float playerAngle;
int oldAngle;
bool isDDA = false;
bool showMinimap = true;
RectangleShape playerRect(vec2(10, 10));
vec2 position;
vec2 vel = vec2(0, 0);
string toString(int n);
Keyboard::Key heldKey;
Texture enemyTexture;
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
{1,0,0,0,0,0,0,0,1,1,1,1,0,1,1,1},
{1,0,0,0,0,0,0,0,1,0,0,1,0,0,0,1},
{1,0,0,0,1,0,0,1,1,1,0,1,1,1,0,1},
{1,0,0,0,1,0,0,0,1,0,0,1,1,0,0,1},
{1,0,1,1,1,0,0,0,1,0,0,1,0,0,1,1},
{1,0,0,0,1,0,0,0,0,0,0,1,0,0,0,1},
{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

vec2 offset = vec2(900 - mapArray.size() * mapSize, 0);
Enemy e1(vec2(40, 60));
int speed=4000;
float precision = 1;
void drawMiniMap(vector<vector<int>> mapA, vec2 offset, int mapSize);
void drawMap(vector<float> distances, vector<int> sides,RenderWindow& window);
void postProcessing(vector<int> arr, int treshold);
float clamp(float val, float minval, float maxval);
float dirAngle;
void start()
{
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
    e1.setTarget(mapArray, position);


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
            vel.y += cos(dir * sens + fov/2*rad) * speed* dt;
            break;
        case(Keyboard::Q):
            vel.x += sin(dir * sens+ (180 - toAngle) * rad) * speed * dt;
            vel.y += cos(dir * sens+ (180 - toAngle) * rad) * speed * dt;
            break;
        case(Keyboard::S):
            vel.x += sin(dir * sens+ (180+toAngle)*rad) *speed * dt;
            vel.y += cos(dir * sens+ (180+toAngle)*rad) *speed * dt;
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
        //-------------------------draw Enemy and objects-------------------------------------

        static VertexArray line(LinesStrip, 2);
        vector<Ray> rays;
        vector<float> distances;
        vector<VertexArray> lines;
        vector<float> directions;
        vector<int> sides;
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
                vec2 fixedPos = vec2((position.x - offset.x),
                                     (position.y - offset.y));
                pair<float, int> result = ray.cast(fixedPos, vec2(dirVec.x, dirVec.y), mapSize, 20, mapArray);
                correctedDist = result.first;

                sides.push_back(result.second);
                lines.push_back(line);
                line[1].position = vec2(position + dirVec * correctedDist);
                distances.push_back(correctedDist * 50);
            }
            else
            {
                Ray ray(line[0].position, dirVec, 100);
                ray.Cast(mapArray, mapSize, offset ,precision);
                line[1].position = vec2(ray.currentPoint.x,ray.currentPoint.y);
                correctedDist = ray.getDist() - (cos(dir*sens+(((float)fov/raycount * i)*rad)));
                sides.push_back(ray.side);
                lines.push_back(line);
                distances.push_back(correctedDist);
            }
        }
        e1.update(dt);
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
        drawMap(distances, sides, window);
        if(showMinimap)
        {
            drawMiniMap(mapArray, offset, 20);
            for(auto& l: lines)
                window.draw(l);
            e1.display(window, offset);
        }
        RectangleShape enemy;
        enemy.setTexture(&enemyTexture, false);
        vec2 eVec = vec2(e1.pos.x + offset.x - position.x,
                         e1.pos.y + offset.y - position.y);
        float eAngle = atan2(eVec.y, eVec.x) * deg + 180;
        float dist = sqrt(eVec.x * eVec.x + eVec.y * eVec.y);
        float startAngle = atan2(cos(dir * sens), sin(dir * sens)) * deg + 180;
        float endAngle = atan2(cos(dir * sens+ fov * rad), sin(dir * sens + fov * rad)) * deg + 180;
        float percent = (eAngle - startAngle) / (endAngle - startAngle);
        cout << percent << endl;
        float maxSize = 400;
        dist *= 2;
        enemy.setSize(vec2(maxSize - dist * 2, maxSize - dist * 2));
        enemy.setOrigin(vec2(enemy.getSize().x / 2, enemy.getSize().y / 2));
        enemy.setPosition(vec2(window.getSize().x - percent * window.getSize().x , 512));
        float col =  clamp(255 - dist, 0, 255);
        enemy.setFillColor(Color(col, col, col));
        if(percent < 1 && percent > 0)
            window.draw(enemy);

        window.display();
    }
    return 0;
}

int fps()
{
    static int t1 = clock();
    static int fps= 0;
    static int lastFps;

    if(t1 - clock() <=-10)
    {
        lastFps = fps * 100;
        fps = 0;
        t1 = clock();
    }
    else
    {
        fps++;
    }
    return lastFps;
}

void drawMap(vector<float> distances, vector<int> sides, RenderWindow& window)
{
    RectangleShape wall;
    wall.setPosition(window.getSize().x, 0);
    int i =0;
    for(auto& d : distances)
    {
        wall.setSize(vec2(window.getSize().x / distances.size(), window.getSize().y - d * 4 ));
        wall.setPosition(wall.getPosition().x, d * 2);
        float shadowPower= 1;
        wall.setFillColor(Color(255,255,255,255-d*shadowPower));
        switch(sides[i])
        {
        case(0):
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
            break;
        }
        if(d*shadowPower >180)
            wall.setFillColor(Color::Black);
        window.draw(wall);
        wall.move(-wall.getSize().x, 0);
        wall.setFillColor(Color(0,0,0));
        i++;
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
