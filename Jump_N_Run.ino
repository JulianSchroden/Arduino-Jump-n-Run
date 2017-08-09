#include <Adafruit_SSD1306.h>
#define ESP32LED 1
#define BUTTON 19
#define INSTANCE_COUNT 5

Adafruit_SSD1306 display(ESP32LED);


class SquareGameObject{
  protected:
    int16_t xPos, yPos;
    int8_t xVel, yVel;
    uint16_t width, height;
    uint16_t color;

  public:
    SquareGameObject(uint16_t _xPos, uint16_t _yPos,int8_t _xVel, int8_t _yVel, uint16_t _width, uint16_t _height,uint16_t _color=WHITE)
    :xPos(_xPos),yPos(_yPos),xVel(_xVel),yVel(_yVel),width(_width),height(_height), color(_color){

    }
    
    int16_t getXPos(){
      return xPos;
    }
    int16_t getYPos(){
      return yPos;
    }
    
    uint16_t getWidth(){
      return width;
    }

    uint16_t getHeight(){
      return height;
    }
    
    
    virtual void draw()=0;
    virtual void update()=0;
    virtual void reset()=0;
};

class Player: public SquareGameObject {
  private:
    int16_t xPosLimit=display.width()/3;

  public:
    Player(uint16_t _width, uint16_t _height,uint16_t _color)
    :SquareGameObject(0,0,5,0,_width, _height,_color){
      
    }
    virtual void draw(){
      xPos=constrain(xPos, 0, display.width()-width);
      yPos=constrain(yPos, 0, display.height()-height);
      display.fillRect(xPos,display.height()-height-yPos, width, height, color);
    }

    virtual void update(){
      if(xPos<xPosLimit)
         xPos+=xVel;
      
      yPos+=yVel;
      if(yPos>0)
        yVel=yVel-2;
    }
    virtual void reset(){
      xPos=0;
      yPos=0;
    }

    void jump(){
      if(yPos==0)
        yVel=9;
    }
};

class Obstacle:public SquareGameObject{
  private:
    boolean hit=false;
  public:
    Obstacle(int16_t _xPos,uint16_t _width, uint16_t _height,uint16_t _color=WHITE)
    :SquareGameObject(_xPos,0,-5,0,_width, _height,_color){
      
    }
    virtual void draw(){
      if(!hit)
        display.drawRect(xPos,display.height()-height-yPos, width, height, color);
      else
        display.fillRect(xPos,display.height()-height-yPos, width, height, color);
    }

    virtual void update(){
      xPos+=xVel;
      if((xPos+width)<0){
        switch (random(0,3)){
          case 0:
            width=6;
            height=12;
            break;
          case 1:
             width=8;
            height=8;
            break;
          case 2:
            width=16;
            height=8;
            break;
          case 3:
            width=24;
            height=6;
            break;
        }
        xPos=display.width()+random(0,display.width());
        hit=false;
      }
    }
     virtual void reset(){
       xVel=-5;
       hit=false;
       xPos+=display.width();
    }
    
    boolean hits(SquareGameObject &player){
      if(!hit && ((player.getXPos()+player.getWidth()) > (this->xPos)) && ((player.getXPos()+player.getWidth()) < (this->xPos+this->width)) && (player.getYPos() < (this->yPos + this->height))){
         hit=true;
         return true;
      }
      return false;
    }

    void stop(){
      xVel=0;
    }
};


Player *player;
Obstacle *obstacles[INSTANCE_COUNT];
int lifeCounter=5;
bool resetFlag=false;

void onButtonClick(){
  if(lifeCounter>0){
    if(resetFlag)
      resetFlag=false;
    else 
      player->jump();
  }
  else{
    resetFlag=true;
  }
}



void drawLifeCounter(){
  display.setCursor((display.width()/2)-3,10);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.println(lifeCounter);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(BUTTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON),onButtonClick, FALLING);
 
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(1000);
  display.clearDisplay();
  
  player=new Player(8,8,WHITE);
  player->draw();
  
  obstacles[0]=new Obstacle(display.width(),8,8);
  obstacles[1]=new Obstacle(display.width()+random(30,display.width()),10,4);
  obstacles[2]=new Obstacle(display.width()*2+random(30,display.width()),16,6);
  for(int i=0; i< INSTANCE_COUNT && obstacles[i]; i++){
    obstacles[i]->draw();
  }
  
  drawLifeCounter();
  display.display();
}


void loop() {
    if(lifeCounter>0){
      display.clearDisplay();
      
      player->update();
      for(int i=0; i< INSTANCE_COUNT && obstacles[i]; i++){
        obstacles[i]->update();
      }
     
      for(int i=0; i< INSTANCE_COUNT && obstacles[i]; i++){
        if(obstacles[i]->hits(*player))
          lifeCounter--;
      }
      if(lifeCounter==0){
        for(int i=0; i< INSTANCE_COUNT && obstacles[i]; i++)
          obstacles[i]->stop();
       
        display.clearDisplay();
        display.setCursor(10,30);
        display.setTextColor(WHITE);
        display.setTextSize(2);
        display.println("new Game?");
        display.display();
        return;
      }
      
      drawLifeCounter();
      player->draw();
      for(int i=0; i< INSTANCE_COUNT && obstacles[i]; i++){
       obstacles[i]->draw();
      }
      display.display();
    }
    else{
      if(resetFlag){
        player->reset();
        for(int i=0; i< INSTANCE_COUNT && obstacles[i]; i++){
          obstacles[i]->reset();
        }
        display.clearDisplay();
        lifeCounter=5;
      }
    }
}
