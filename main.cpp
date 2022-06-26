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
}

class Texture{
  private:
    sf::Texture player_;
    sf::Texture alien_;
  public:
    Texture(){
      // Codes: Portal, spiderman/ironman
      if(!this->player_.loadFromFile("resources/plane.png")){
        throw "Unable to load ship texture";
      }
      if(!this->alien_.loadFromFile("resources/alien.png")){
        throw "Unable to load alien texture";
      }
    }
    sf::Texture* GetPlayer(){
      return &this->player_;
    }
    sf::Texture* GetAlien(){
      return &this->alien_;
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
    void Move(sf::Sprite &sprite, Direction direction){
      switch(direction){
        case Direction::kUp: 
          sprite.move(0,-10);
          break;
        case Direction::kRight:
          sprite.move(10,0);
          break;
        case Direction::kDown:
          sprite.move(0,10);
          break;
        case Direction::kLeft:
          sprite.move(-10,-0);
          break;
        default:
          std::cout << "Wrong direction\n";
      }
    }
};

class Player: protected Movable{
  private:
    sf::Sprite sprite_;
    const int player_height_ = 150;
  public:
    Player(sf::Texture *player_texture, unsigned int screen_width, unsigned int screen_height){
      this->sprite_.setTexture(*player_texture);
      this->sprite_.setOrigin(this->sprite_.getLocalBounds().width/2, this->sprite_.getLocalBounds().height/2);
      float scale = GetScale(this->sprite_.getGlobalBounds().height, this->player_height_);
      this->sprite_.setScale(scale, scale); 
      this->sprite_.setPosition(screen_width*0.5, screen_height*0.7);
    }

    void Update(sf::RenderWindow &window){
      if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left)){
        Movable::Move(this->sprite_, Direction::kLeft);
      }
      if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right)){
        Movable::Move(this->sprite_, Direction::kRight);
      }
      window.draw(this->sprite_);
    }
    
};

class Alien{
  private: 
    sf::Sprite sprite_;
    const int alien_height_ = 100;
  public:
    Alien(sf::Texture *alien_texture){
      this->sprite_.setTexture(*alien_texture);
      this->sprite_.setOrigin(this->sprite_.getLocalBounds().width/2, this->sprite_.getLocalBounds().height/2);
      float scale = GetScale(this->sprite_.getGlobalBounds().height, this->alien_height_);
      this->sprite_.setScale(scale, scale);
      this->sprite_.setPosition(this->sprite_.getGlobalBounds().width, this->sprite_.getGlobalBounds().height);
    }

    void Draw(sf::RenderWindow &window){
      window.draw(this->sprite_);
    }
};

int main(){
  // ToDo check if real improvement
  std::vector<sf::VideoMode> * modes = new std::vector<sf::VideoMode>{sf::VideoMode::getFullscreenModes()};
  //const unsigned int screen_width = 800;
  //const unsigned int screen_height = 400;
  const unsigned int screen_width = (*modes)[0].width;
  const unsigned int screen_height = (*modes)[0].height;
  delete modes;
  sf::RenderWindow window(sf::VideoMode(screen_width,screen_height), ".");
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

  Player player(textures->GetPlayer(), screen_width, screen_height);
  Alien alien(textures->GetAlien());
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
      window.clear(sf::Color::White);
   
      player.Update(window); 
      alien.Draw(window);

      window.display();
    }
  }
  
  return 0;
}
