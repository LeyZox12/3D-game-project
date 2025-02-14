#include <iostream>
#include <algorithm>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
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
            if(!isTargetSet && mat[target.y][target.x] != 1)
            {
                mat[target.y][target.x] = 3;
                currentCase = vec2(floor((pos.x) / mapSize), floor((pos.y) / mapSize));
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
                    pos += vec2(actions[actionIndex].x * mapSize,
                                actions[actionIndex].y * mapSize);
                    actionIndex++;
                }
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
        int interact()
        {

            for(int i = 0; i < objects.size(); i++)
            {

                if(objects[i].getInteractible())
                {
                    string name = objects[i].getName();
                    cout << name << endl;
                    if(name == "slotMachine")
                    {
                        return 0;
                    }
                    else if(name == "blackJack")
                    {
                        return 1;
                    }
                    return -1;
                }
            }
            return -1;
        }
        void addObject(vec2 pos, Texture texture, int yPos, string name)
        {
            object newObj;
            newObj.setTexture(texture);
            newObj.setPosition(pos);
            newObj.setYPos(yPos);
            newObj.setName(name);
            objects.push_back(newObj);
        }
        void setPos(int index, float x, float z)
        {
            objects[index].setPosition(vec2(x, z));
        }
        void setY(int index, float y)
        {
            objects[index].setYPos(y);
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
        void addInteractibleCount(int i_offset)
        {
            this -> interactibleCount += i_offset;
        }

    private:
        class object
        {
            public:
                object()
                {

                }
                void setName(string name)
                {
                    this -> name = name;
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
                    this -> texture = texture;
                }
                void setInteractible(bool interactible)
                {
                    this -> interactible = interactible;
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
                string getName()
                {
                    return this -> name;
                }
                bool getInteractible()
                {
                    return this -> interactible;
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
                    //float rSize = dist * 2 < 0 ? 0 : (dist * 2 > maxSize ? maxSize : dist * 2);
                    vec2 endSize = vec2(maxSize / (dist / 50), maxSize / (dist / 50));
                    vec2 endPos = vec2(percent * window.getSize().x, yPos);
                    //centers the sprite
                    sprite.move(vec2(dist / 2, dist / 2));
                    float col = 255 / (dist / 100);
                    col =  col > 255 ? 255 : (col < 0 ? 0 : col);
                    sprite.setFillColor(Color(col, col, col));
                    if(eAngle > startAngle && eAngle < endAngle)
                    {
                        if(dist < 40 && !getInteractible())
                        {
                            setInteractible(true);
                            cout << getName() << ": true\n";
                        }
                        else if(dist > 40 && getInteractible())
                        {
                            cout << getName() << ": false\n";
                            setInteractible(false);
                        }
                        sprite.setSize(endSize);
                        sprite.setPosition(endPos);
                        sprite.setTexture(&texture);
                        window.draw(sprite);
                        setBeenDrawn(true);
                    }
                    else
                        setInteractible(false);
                }
            private:
                bool interactible = false;
                Texture texture;
                bool hasBeenDrawn;
                vec2 pos;
                float yPos;
                string name;
        };
        vector<object> objects;
        int interactibleCount = 0;
};

RenderWindow window(VideoMode::getDesktopMode(), "Raycaster", Style::Fullscreen);
Event e;
Font font;
Text fpsText;
bool isDDA = true;
bool showMinimap = true;
bool isDebug = false;
int gamePlayed = -1;
bool playingCasino = false;
int gameTimeStart;
bool tryInteract = false;
int fps();
int fov = 90;
int mapSize = 20;
int raycount;
float sens = 0.0025;
float oldAngle;
float playerAngle;
void start();
long double dt;
long double x;
const double rad = 0.0174533;
const double deg = 57.2958;
double dir = 0;

typedef Vector2f vec2;

Sound sound;
CircleShape player(5);
vec2 position;
vec2 vel = vec2(0, 0);
string toString(int n);
vector<Keyboard::Key> heldKeys;
Texture enemyTexture;
Texture floorTexture;
Texture slotTexture;
Texture slotSheet;
SoundBuffer scream;
vector<vector<int>> mapArray = {
{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
{1,0,0,1,0,0,0,0,1,1,0,0,0,0,0,1},
{1,0,0,1,0,0,0,0,0,1,1,1,1,1,1,1},
{1,0,0,1,1,1,0,0,0,0,1,0,0,0,0,1},
{1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1},
{1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1},
{1,0,0,1,1,0,0,0,0,0,1,0,0,0,0,1},
{1,0,0,0,1,1,0,0,0,0,1,0,0,0,0,1},
{1,0,0,0,0,0,0,0,1,0,1,0,0,0,0,1},
{1,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1},
{1,0,0,0,1,0,0,0,1,1,0,1,1,1,0,1},
{1,0,0,0,1,0,0,0,1,0,0,1,0,0,0,1},
{1,0,1,1,1,0,0,0,1,0,0,0,0,0,0,1},
{1,0,0,0,1,0,0,0,0,0,0,1,0,0,0,1},
{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

vector<vector<int>> textureMap = {
{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
{1,0,0,1,0,0,0,0,1,1,0,0,0,0,0,1},
{1,0,0,1,0,0,0,0,0,1,2,2,2,2,2,2},
{1,0,0,1,1,1,0,0,0,0,2,0,0,0,0,2},
{1,0,0,0,0,1,0,0,0,0,2,0,0,0,0,2},
{1,0,0,0,0,0,0,0,0,0,2,0,0,0,0,2},
{1,0,0,1,1,0,0,0,0,0,2,0,0,0,0,2},
{1,0,0,0,1,1,0,0,0,0,2,0,0,0,0,2},
{1,0,0,0,0,0,0,0,1,0,2,0,0,0,0,2},
{1,0,0,0,0,0,0,0,0,0,2,2,0,0,0,2},
{1,0,0,0,1,0,0,0,1,1,0,2,2,2,0,2},
{1,0,0,0,1,0,0,0,1,0,0,2,0,0,0,2},
{1,0,1,1,1,0,0,0,1,0,0,0,0,0,0,2},
{1,0,0,0,1,0,0,0,0,0,0,2,0,0,0,2},
{1,1,1,1,1,1,1,1,1,1,1,2,2,3,2,2}
};
class player
{
    public:
        player(vec2 pos)
        {
            this -> position = pos;
            this -> money = 100.f;
        }
        float getMoney()
        {
            return this -> money;
        }
        vec2 getPosition()
        {
            return position;
        }
        void setPosition(vec2 pos)
        {
            this -> position = pos;
        }
        void setMoney(float m)
        {
            this -> money = m;
        }
    private:
        vec2 position;
        vec2 vel;
        vector<int> items;
        float money;
        int index;
};
vec2 offset = vec2(900 - mapArray.size() * mapSize, 0);
Enemy e1(vec2(mapSize, mapSize));
objManager obj;
int speed=500;
float precision = 1;
float dirAngle;
void drawMiniMap(vector<vector<int>> mapA, vec2 offset, int mapSize);
void drawMap(vector<float> distances, vector<int> sides, vector<float> percents, vector<int> textureIndexes, RenderWindow& window);
void postProcessing(vector<int> arr, int treshold);
void updateGame(RenderWindow& window, int& gamePlaying);
float clamp(float val, float minval, float maxval);
int ballRectCollision(CircleShape ball, RectangleShape rect);
void start()
{
    window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(60);
    floorTexture.loadFromFile("res/floor.png");
    slotTexture.loadFromFile("res/slot_machine.png");
    enemyTexture.loadFromFile("res/freaky.png");
    scream.loadFromFile("res/scream.wav");
    slotSheet.loadFromFile("res/slotSprites.png");
    sound.setBuffer(scream);
    sound.play();
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
    obj.addObject(e1.pos, enemyTexture, window.getSize().y / 2, "enemy");
    obj.addObject(vec2(227, 187), slotTexture, window.getSize().y/2, "slotMachine");

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
    float time = clock();
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
                        heldKeys.push_back(Keyboard::Z);
                        break;
                    case(Keyboard::Q):
                        heldKeys.push_back(Keyboard::Q);
                        break;
                    case(Keyboard::S):
                        heldKeys.push_back(Keyboard::S);
                        break;
                    case(Keyboard::D):
                        heldKeys.push_back(Keyboard::D);
                        break;
                    case(Keyboard::E):
                        if(obj.interact() > -1 && !playingCasino)
                        {
                            gamePlayed = obj.interact();
                            playingCasino = true;
                            gameTimeStart = clock();
                        }
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
                    case(Keyboard::F3):
                        isDebug = !isDebug;
                        break;
                }
            }
            if(e.type == Event::KeyReleased)
                if(e.key.code == Keyboard::Z ||
                   e.key.code == Keyboard::Q ||
                   e.key.code == Keyboard::S ||
                   e.key.code == Keyboard::D)
                    heldKeys.erase(find(heldKeys.begin(), heldKeys.end(), e.key.code));
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
        if(raycount < 0)
            raycount = fov;
        vec2 diff = position - offset - e1.pos;
        float dist = sqrt(diff.x * diff.x + diff.y * diff.y);
        sound.setVolume(20 / (dist / 5));
        sound.setLoop(true);

        obj.setY(0, window.getSize().y / 2);
        if (fps() !=0)
            dt=(float)1/60;
        window.clear(Color::Black);
        int toAngle = (180-fov )/2;

        for(int i = 0; i < heldKeys.size(); i++)
            switch(heldKeys[i])
            {
            case(Keyboard::Z):
                vel.x += (sin(dir *sens + fov / 2 * rad) * speed * dt) / heldKeys.size();
                vel.y += (cos(dir * sens + fov / 2 *rad) * speed* dt) / heldKeys.size();
                break;
            case(Keyboard::Q):
                vel.x += (sin(dir * sens+ (180 - toAngle) * rad) * speed * dt) / heldKeys.size();
                vel.y += (cos(dir * sens+ (180 - toAngle) * rad) * speed * dt) / heldKeys.size();
                break;
            case(Keyboard::S):
                vel.x += (sin(dir * sens+ (180 + fov / 2)*rad) *speed * dt) / heldKeys.size();
                vel.y += (cos(dir * sens+ (180 + fov / 2)*rad) *speed * dt) / heldKeys.size();
                break;
            case(Keyboard::D):
                vel.x += (sin(dir*sens - toAngle*rad)* speed * dt) / heldKeys.size();
                vel.y += (cos(dir*sens - toAngle*rad)* speed * dt) / heldKeys.size();
                break;
            }
        window.draw(fpsText);
        static VertexArray line(LinesStrip, 2);
        vector<Ray> rays;
        vector<float> distances;
        vector<VertexArray> lines;
        vector<float> directions;
        vector<int> sides;
        vector<float> percents;
        vector<int> textureIndexes;
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
                textureIndexes.push_back(textureMap[ray.getPosition().y][ray.getPosition().x]);
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
        }
        obj.setPos(0, e1.pos.x, e1.pos.y);
        e1.update(dt);
        player.setPosition(position);
        player.setOrigin(player.getRadius(), player.getRadius());
        for(int i = 0; i < mapArray.size(); i++)
        {
            for(int j = 0; j < mapArray[i].size(); j++)
            {
                if(mapArray[i][j] == 1)
                {
                    RectangleShape r2(vec2(mapSize, mapSize));
                    r2.setPosition(j * mapSize + offset.x, i * mapSize + offset.y);
                    int result = ballRectCollision(player, r2);
                    switch(result)
                    {
                    case(0):
                        position = vec2(position.x, i * mapSize - player.getRadius() + offset.y);
                        vel.y = 0;
                        break;
                    case(1):
                        position = vec2(position.x, i * mapSize + mapSize + player.getRadius() + offset.y);
                        vel.y = 0;
                        break;
                    case(2):
                        position = vec2(j * mapSize - player.getRadius() + offset.x, position.y);
                        vel.x = 0;
                        break;
                    case(3):
                        vel.x = 0;
                        position = vec2(j * mapSize + mapSize + player.getRadius() + offset.x, position.y);
                        break;
                    }
                }
            }
        }
        position += vec2(vel.x * dt, vel.y * dt);
        vel = vec2(vel.x * 0.9, vel.y * 0.9);
        drawMap(distances, sides, percents, textureIndexes, window);
        if(showMinimap)
        {
            drawMiniMap(mapArray, offset, 20);
            for(auto& l: lines)
                window.draw(l);
            e1.display(window, offset);
        }
        if(gamePlayed > -1)
            updateGame(window, gamePlayed);
        if(isDebug)
        {
            Text debugText;
            debugText.setFont(font);
            debugText.setString("pos\nx:" + toString(position.x - offset.x) +
                                "\ny:" + toString(position.y - offset.y) +
                                "\nrayCount:" + toString(raycount) +
                                "\nfov:" + toString(fov) +
                                "\ncasinoGameIndex:" + toString(gamePlayed));
            window.draw(debugText);
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
void updateGame(RenderWindow& window, int& gamePlaying)
{
    switch(gamePlaying)
    {
    case(0):
        {
            RectangleShape rect;
            rect.setTexture(&slotSheet);
            rect.setTextureRect({50, 0, 111, 67});
            vector<RectangleShape> slots;
            static vector<int> indexes;
            static vector<int> randomOffsets = {rand(), rand(), rand()};
            static int current = 0;
            slots.push_back(RectangleShape());
            slots.push_back(RectangleShape());
            slots.push_back(RectangleShape());
            rect.setSize(vec2(window.getSize().x / 2, window.getSize().y / 2));
            float slotRatioX =rect.getSize().x / 111;
            float slotRatioY =rect.getSize().y / 67;


            rect.setPosition(window.getSize().x / 4,
                             window.getSize().y / 4);
            window.draw(rect);
            if(clock() - gameTimeStart > 3000 + 1000 * current)
            {
                int index = floor(fmod((randomOffsets[current] + (clock() - gameTimeStart) * 0.5), 150) / 50);
                cout << index << endl;
                indexes.push_back(index);
                current++;
            }
            if(clock() - gameTimeStart > 8000)
            {
                bool hasWon = true;
                for(int i = 0; i < 3; i++)
                    if(indexes[i] != indexes[0])
                    {
                        hasWon = false;
                        break;
                    }
                string result = hasWon ? "you won!!\n" : "lost\n";
                cout << "result:" << result;
                randomOffsets = {rand() % 150, rand() % 150, rand() % 150};
                gamePlaying = -1;
                current = 0;
                playingCasino = false;
                indexes.clear();
            }
            else
            {
                slotSheet.setRepeated(true);
                for(int i = 0; i < 3; i++)
                {
                    slots[i].setSize(vec2(slotRatioX * 25, slotRatioY * 50));
                    slots[i].setPosition(vec2(rect.getPosition().x + slotRatioX * i * 37 + slotRatioX * 6, rect.getPosition().y + slotRatioY * 9));
                    slots[i].setTexture(&slotSheet);
                    slots[i].setTextureRect({25, 0, 25, 50});
                    window.draw(slots[i]);
                    int y = fmod((randomOffsets[i] + (clock() - gameTimeStart) * 0.5), 150);
                    slots[i].setTextureRect({0, current > i ? indexes[i] * 50 : y, 25, 50});
                    window.draw(slots[i]);
                    slots[i].setTextureRect({25, 50, 25, 50});
                    window.draw(slots[i]);
                }
            }
            break;
        }
    case(1):
        break;
    }

}
void drawMap(vector<float> distances, vector<int> sides, vector<float> percents, vector<int> textureIndex, RenderWindow& window)
{
    double ratioAngle = distances.size() / fov;
    Texture wallTexture;
    wallTexture.loadFromFile("res/spriteSheet.png");
    Vector2u textureSize = wallTexture.getSize();
    wallTexture.setRepeated(true);
    RectangleShape wall;
    wall.setPosition(window.getSize().x, 0);
    wall.setTexture(&wallTexture);

    vector<pair<float, int>> distancesSorted;
    for(int i = 0; i < distances.size(); i++)
    {
        distancesSorted.push_back(make_pair(distances[i], i));
    }
    sort(distancesSorted.begin(), distancesSorted.end());
    reverse(distancesSorted.begin(), distancesSorted.end());
    for(auto& d : distancesSorted)
    {
        //---------------wall drawing------------------------

        int index = d.second;
        wall.setPosition(window.getSize().x - index * window.getSize().x / distances.size(), 0);
        float dist = distances[index];
        wall.setSize(vec2(window.getSize().x / distances.size() + 1, window.getSize().y / (dist / 50)));
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
            wall.setTextureRect({percents[index] * 32 + (textureIndex[index] - 1) * 32, 0, wall.getSize().x / 32, 32});
        Color color = wall.getFillColor();
        color.r = clamp(color.r, 0, 255);
        color.g = clamp(color.g, 0, 255);
        color.b = clamp(color.b, 0, 255);
        wall.setFillColor(color);
        obj.drawObjects(window, dist, position - offset, dir * sens, fov, 400);
        window.draw(wall);
        //-------------------ceil drawing---------------------
        RectangleShape ceil(vec2(wall.getSize().x, wall.getPosition().y));
        ceil.setPosition(wall.getPosition().x, 0);
        ceil.setFillColor(Color::Black);
        window.draw(ceil);
        wall.setFillColor(Color(0,0,0));
    }
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
    window.draw(player);
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
int ballRectCollision(CircleShape ball, RectangleShape rect)
{
    //choose correct side;
    vec2 rectPos = rect.getPosition();
    vec2 rectSize = rect.getSize();
    vec2 ballPos = ball.getPosition();
    float x = ball.getPosition().x;
    float y = ball.getPosition().y;
    if(x<rect.getPosition().x)
    {
        x = rect.getPosition().x;

    }
    else if(x>rect.getPosition().x + rect.getSize().x)
        x = rect.getPosition().x + rect.getSize().x;
    if(y<rect.getPosition().y)
    {
        y = rect.getPosition().y;
    }
    else if(y>rect.getPosition().y+rect.getSize().y)
        y = rect.getPosition().y + rect.getSize().y;
    Vector2f diff = Vector2f(ball.getPosition().x - x, ball.getPosition().y -y);
    float dist = sqrt(diff.x*diff.x+ diff.y*diff.y);
    if(dist<ball.getRadius())
    {
        int radius = ball.getRadius();
        vector<float> distances = {
                                abs(rectPos.y - (ballPos.y + radius)),
                                abs(rectPos.y + rectSize.y - (ballPos.y - radius)),
                                abs(rectPos.x - (ballPos.x + radius)),
                                abs(rectPos.x + rectSize.x - (ballPos.x - radius))
                                };
        auto it = min_element(distances.begin(), distances.end());
        int index = distance(distances.begin(), it);
        return index;
    }
    return -1;

}
