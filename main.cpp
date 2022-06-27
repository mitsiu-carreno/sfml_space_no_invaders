// g++ main.cpp -o main -I SFML-2.5.1/include/ -L SFML-2.5.1/lib/ -lsfml-graphics -lsfml-window -lsfml-system  && export LD_LIBRARY_PATH=SFML-2.5.1/lib/  && ./main
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>

float GetScale(float initial_measure, float target_measure){
  return target_measure/initial_measure;
}

namespace constants{
  constexpr int kFrameRate = 60;  // Frames per second
  constexpr int kFrameDuration = 1000/kFrameRate;
  constexpr int kScreenMargins = 10;
}

class Texture{
  private:
    sf::Texture player_;
    sf::Texture alien_;
    sf::Texture rocket_;
  public:
    Texture(){
      // Codes: Portal, spiderman/ironman, game mechanic elites desapear on stall
      if(!this->player_.loadFromFile("resources/plane.png")){
        throw "Unable to load ship texture";
      }
      if(!this->alien_.loadFromFile("resources/alien.png")){
        throw "Unable to load alien texture";
      }
      if(!this->rocket_.loadFromFile("resources/rocket.png")){
        throw "Unable to load rocket texture";
      }
    }
    sf::Texture* GetPlayer(){
      return &this->player_;
    }
    sf::Texture* GetAlien(){
      return &this->alien_;
    }
    sf::Texture* GetRocket(){
      return &this->rocket_;
    }
};

class Movable{
  protected:
    enum class Direction{
      kUp,
      kRight,
      kDown,
      kLeft,
    };
    void Move(sf::Sprite &sprite, Direction direction, float distance){
      switch(direction){
        case Direction::kUp: 
          sprite.move(0,-distance);
          break;
        case Direction::kRight:
          sprite.move(distance,0);
          break;
        case Direction::kDown:
          sprite.move(0,distance);
          break;
        case Direction::kLeft:
          sprite.move(-distance,0);
          break;
        default:
          std::cout << "Wrong direction\n";
      }
    }
};

class Bullet: protected Movable{
  private:
    sf::Sprite sprite_;
    const int bullet_height_ = 40;
    const float speed_ = 0.6;   // pixels / milliseconds
    bool friendy;
  public:
    Bullet(bool friendly, sf::Texture& texture, float pos_x, float pos_y){
      this->sprite_.setTexture(texture);
      this->sprite_.setOrigin(this->sprite_.getLocalBounds().width/2, this->sprite_.getLocalBounds().height/2);
      float scale = GetScale(this->sprite_.getGlobalBounds().height, this->bullet_height_);
      this->sprite_.setScale(scale, scale);
      this->sprite_.setPosition(pos_x, pos_y);
    }
    void Update(sf::RenderWindow &window, int elapsed){
      Movable::Move(this->sprite_, Movable::Direction::kUp, elapsed * this->speed_);
      window.draw(this->sprite_);
    }

};

class Player: protected Movable{
  private:
    sf::Sprite sprite_;
    const int player_height_ = 150; // Pixels
    const float speed_ = 0.625;      // pixels / millisecond
    sf::Texture *bullet_texture_;
    std::vector<Bullet*> bullets_;    // todo deconstructor
    const int fire_cooldown = 100;
    int remaining_fire_cooldown = fire_cooldown; 
  public:
    Player(sf::Texture *player_texture, unsigned int screen_width, unsigned int screen_height, sf::Texture *bullet_texture){
      this->sprite_.setTexture(*player_texture);
      this->sprite_.setOrigin(this->sprite_.getLocalBounds().width/2, this->sprite_.getLocalBounds().height/2);
      float scale = GetScale(this->sprite_.getGlobalBounds().height, this->player_height_);
      this->sprite_.setScale(scale, scale); 
      this->sprite_.setPosition(screen_width*0.5, screen_height*0.7);

      this->bullet_texture_ = bullet_texture;
    }

    void Update(sf::RenderWindow &window, int elapsed){
      if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left)){
        if(this->sprite_.getGlobalBounds().left > constants::kScreenMargins){
          Movable::Move(this->sprite_, Direction::kLeft, elapsed * this->speed_);
        }
      }
      if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right)){
        if(this->sprite_.getGlobalBounds().left + this->sprite_.getGlobalBounds().width < window.getSize().x - constants::kScreenMargins){
          Movable::Move(this->sprite_, Movable::Direction::kRight, elapsed * this->speed_);
        }
      }
      if(sf::Keyboard::isKeyPressed(sf::Keyboard::Space)){
        if(this->remaining_fire_cooldown == 0){
          this->Fire(); 
          this->remaining_fire_cooldown = this->fire_cooldown;
        }else{
          // Lock minimum value to 0
          this->remaining_fire_cooldown = (this->remaining_fire_cooldown < 0)? 0 : this->remaining_fire_cooldown - elapsed; 
        }
      }

      for(Bullet *bullet : this->bullets_){
        bullet->Update(window, elapsed);
      }
      window.draw(this->sprite_);
    }

    void Fire(){
      this->bullets_.push_back(
          new Bullet(true, *(this->bullet_texture_), this->sprite_.getPosition().x, this->sprite_.getGlobalBounds().top)
      );
      //Bullet new_bullet = Bullet(true, *(this->bullet_texture_), this->sprite_.getPosition());  
    }
    
};

class Alien: protected Movable{
  private: 
    sf::Sprite sprite_;
    const int alien_height_ = 100;        // Pixels
    const float speed_ = 0.3125;           // Pixels / millisecond
  public:
    Alien(sf::Texture *alien_texture, int soldier_num){
      this->sprite_.setTexture(*alien_texture);
      this->sprite_.setOrigin(this->sprite_.getLocalBounds().width/2, this->sprite_.getLocalBounds().height/2);
      float scale = GetScale(this->sprite_.getGlobalBounds().height, this->alien_height_);
      this->sprite_.setScale(scale, scale);
      this->sprite_.setPosition(this->sprite_.getGlobalBounds().width * soldier_num, this->sprite_.getGlobalBounds().height);
    }

    void Move(Movable::Direction direction, int elapsed){
      Movable::Move(this->sprite_, direction, elapsed * this->speed_);
    }
    void Draw(sf::RenderWindow &window){
      window.draw(this->sprite_);
    }
};

class AlienCovenant: protected Movable{
  private:
    std::vector<Alien*> covenant_;
    Movable::Direction movement_loop[4];
    int current_direction; 
    const int stall_duration = 1200;  // Milliseconds
    const int movement_duration = 200;        // Milliseconds
    float elapsed_stall = 0;
    float elapsed_movement = 0;
  protected:
    Movable::Direction GetCurrentDirection(){
      return movement_loop[this->current_direction];
    }
    void UpdateDirection(){
      this->current_direction = (this->current_direction+1) % 4;
    }
  public:
    AlienCovenant(){
      // todo Create aliens here?!
      this->movement_loop[0] = Movable::Direction::kRight;
      this->movement_loop[1] = Movable::Direction::kDown;
      this->movement_loop[2] = Movable::Direction::kLeft;
      this->movement_loop[3] = Movable::Direction::kDown;

      this->current_direction = 0;
      this->covenant_.reserve(50);
    }
    
    void Enlist(Alien *new_alien){
      this->covenant_.push_back(new_alien);
    }

    void Update(sf::RenderWindow &window, int elapsed){
      elapsed_stall += elapsed;
      if(elapsed_stall > stall_duration){
        elapsed_movement += elapsed;
        /*
        for(Alien *soldier : this->covenant_){
          soldier->Move(AlienCovenant::GetCurrentDirection(), elapsed);
        }
        */
        if(elapsed_movement > movement_duration){
          elapsed_stall = 0;
          elapsed_movement = 0;
          this->UpdateDirection();
        }
      }
      for(Alien *soldier : this->covenant_){
        // re do movement check to avoid double covenant range-base loop
        if(elapsed_movement > 0 && elapsed_movement <= movement_duration){
          soldier->Move(AlienCovenant::GetCurrentDirection(), elapsed);
        }
        soldier->Draw(window);
      }
    }
};


int main(){
  // todo check if real improvement
  std::vector<sf::VideoMode> * modes = new std::vector<sf::VideoMode>{sf::VideoMode::getFullscreenModes()};
  //const unsigned int screen_width = 800;
  //const unsigned int screen_height = 400;
  const unsigned int screen_width = (*modes)[0].width;
  const unsigned int screen_height = (*modes)[0].height;
  delete modes;
  sf::RenderWindow window(sf::VideoMode(screen_width,screen_height), "Space Invaders!");
  //sf::Vector2i window_offset(0,0);
  //window.setPosition(window_offset);
 
  // Allocate space for Texture object
  Texture *textures {nullptr}; 
  try{
    textures = new Texture();
  }catch(const char *e){
    std::cout << e << "\n";
    delete textures;
    return EXIT_FAILURE;
  }

  Player player(textures->GetPlayer(), screen_width, screen_height, textures->GetRocket());
  AlienCovenant covenant;
  Alien alien1(textures->GetAlien(),1);
  Alien alien2(textures->GetAlien(),2);
  Alien alien3(textures->GetAlien(),3);
  Alien alien4(textures->GetAlien(),4);
  Alien alien5(textures->GetAlien(),5);
  Alien alien6(textures->GetAlien(),6);
  Alien alien7(textures->GetAlien(),7);
  Alien alien8(textures->GetAlien(),8);
  Alien alien9(textures->GetAlien(),9);
  Alien alien10(textures->GetAlien(),10);
  covenant.Enlist(&alien1);
  covenant.Enlist(&alien2);
  covenant.Enlist(&alien3);
  covenant.Enlist(&alien4);
  covenant.Enlist(&alien5);
  covenant.Enlist(&alien6);
  covenant.Enlist(&alien7);
  covenant.Enlist(&alien8);
  covenant.Enlist(&alien9);
  covenant.Enlist(&alien10);
  //////
  sf::Clock clock;
  while(window.isOpen()){
    sf::Event event;
    while(window.pollEvent(event)){
      if(event.type == sf::Event::Closed){
        delete textures;
        textures = nullptr;
        window.close();
      }
    }
    sf::Time elapsed = clock.getElapsedTime();
    if(elapsed.asMilliseconds() >= constants::kFrameDuration){
      clock.restart();
      window.clear(sf::Color(142,142,142));
   
      player.Update(window, elapsed.asMilliseconds()); 
      covenant.Update(window, elapsed.asMilliseconds());

      window.display();
    }
  }
  
  return 0;
}
