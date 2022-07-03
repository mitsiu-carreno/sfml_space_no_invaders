// g++ main.cpp -o main -I SFML-2.5.1/include/ -L SFML-2.5.1/lib/ -lsfml-graphics -lsfml-window -lsfml-system  && export LD_LIBRARY_PATH=SFML-2.5.1/lib/  && ./main
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
// todo vector reserve everywhere

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
    bool friendy;
    sf::FloatRect hitbox_;
  public:
    bool active = true;
    static const float speed_;   // pixels / milliseconds
    static const int bullet_height_;
    Bullet(bool friendly, sf::Texture& texture, float pos_x, float pos_y){
      this->sprite_.setTexture(texture);
      this->sprite_.setOrigin(this->sprite_.getLocalBounds().width/2, this->sprite_.getLocalBounds().height/2);
      float scale = GetScale(this->sprite_.getGlobalBounds().height, this->bullet_height_);
      this->sprite_.setScale(scale, scale);
      this->sprite_.setPosition(pos_x, pos_y);
      this->hitbox_ = {0.f,0.f, this->sprite_.getLocalBounds().width, this->sprite_.getLocalBounds().height};
    }
    void Update(sf::RenderWindow &window, int elapsed){
      Movable::Move(this->sprite_, Movable::Direction::kUp, elapsed * this->speed_);
      window.draw(this->sprite_);
    }
    sf::Vector2f GetPosition(){
      return this->sprite_.getPosition();
    }

    sf::FloatRect GetHitBox(){
      return this->sprite_.getTransform().transformRect(this->hitbox_);
    }



};
constexpr float Bullet::speed_ = 0.6;
constexpr int Bullet::bullet_height_ = 40;

class AlienAccessPlayerMagazine{  // todo rename
  private: 
    std::vector<Bullet>* bullets_;
  public:
    //AlienAccessPlayerMagazine(std::vector<Bullet> *vec): bullets_{vec}{}

    void SetBulletsPointer(std::vector<Bullet> *p){
      this->bullets_ = p;
    }
    std::vector<sf::FloatRect> GetBulletsHitBoxes(){
      std::vector<sf::FloatRect> temp;
      temp.reserve((*this->bullets_).size());
      for(Bullet &bullet : (*this->bullets_)){
        temp.push_back(bullet.GetHitBox());
      }
      return temp;
    }

    void DeleteBullet(unsigned int i){
      (*this->bullets_).erase((*this->bullets_).begin() + i);
    }
};

class BulletMagazine{
  private:
    sf::Texture *bullet_texture_;
    std::vector<Bullet> bullets_;
    AlienAccessPlayerMagazine player_magazine_;
    bool friendly;    // todo adapt friendly here
  public:
    BulletMagazine(unsigned int screen_height, const int fire_cooldown, sf::Texture *bullet_texture){
      this->bullet_texture_ = bullet_texture;
      this->bullets_.reserve((screen_height/Bullet::speed_)/fire_cooldown);
      player_magazine_.SetBulletsPointer(&this->bullets_);
    }
    void Update(sf::RenderWindow &window, int elapsed){
      for(Bullet &bullet : this->bullets_){
        bullet.Update(window, elapsed);
        /* Debug
        sf::RectangleShape rectangle;
        rectangle.setPosition(bullet->GetHitBox().left, bullet->GetHitBox().top);
        rectangle.setSize(sf::Vector2f(bullet->GetHitBox().width, bullet->GetHitBox().height));
        rectangle.setFillColor(sf::Color::Transparent);
        rectangle.setOutlineColor(sf::Color::Red);
        rectangle.setOutlineThickness(3.f);

        window.draw(rectangle);
        */
      }
    }
    void AddBullet(int friendly, float pos_x, float pos_y){
      Bullet *new_bullet = new Bullet(
          friendly, 
          *(this->bullet_texture_),
          pos_x,
          pos_y
      );
      this->bullets_.push_back(*new_bullet);
    }
    /*
    void DeleteBullet(unsigned int i){
      this->bullets_.erase(this->bullets_.begin()+i); 
    }
    */
    void ClearBullets(){
      unsigned i = 0;
      unsigned deleted = -1;
      for(Bullet &bullet : this->bullets_){
        if(bullet.GetPosition().y < 0 - Bullet::bullet_height_ || !bullet.active){
          //std::cout << "Will delete at " << bullet.GetPosition().y << "\n";
          std::cout << "cap" << this->bullets_.capacity() << " size:" << this->bullets_.size() << "\n";
          deleted = i;
        }
        ++i;
      }
      if(deleted != -1){
        this->bullets_.erase(this->bullets_.begin()+deleted);
      }
    }
    /*
    std::vector<sf::FloatRect> GetBulletsHitBoxes(){
      std::vector<sf::FloatRect> temp;
      temp.reserve(this->bullets_.size());
      for(Bullet &bullet : this->bullets_){
        temp.push_back(bullet.GetHitBox());
      }
      return temp;
    }
    */
    AlienAccessPlayerMagazine* GetPlayerMagazine(){
      return &(this->player_magazine_);
    }
};

/*class AlienAccessMagazine:private BulletMagazine{
  public: 
    using BulletMagazine::GetBulletsHitBoxes;
    using BulletMagazine::DeleteBullet;
};
*/

class Player: protected Movable{    // todo Inherit from sprite?
  private:
    sf::Sprite sprite_;
    const int player_height_ = 150; // Pixels
    const float speed_ = 0.625;      // pixels / millisecond
    sf::Texture *bullet_texture_;
    const int fire_cooldown = 300;
    int remaining_fire_cooldown = fire_cooldown; 
    BulletMagazine magazine_;
  public:
    Player(
        sf::Texture *player_texture, 
        unsigned int screen_width, 
        unsigned int screen_height, 
        sf::Texture *bullet_texture
    ):magazine_(
        screen_height, 
        this->fire_cooldown, 
        bullet_texture
     ){
      this->sprite_.setTexture(*player_texture);
      this->sprite_.setOrigin(this->sprite_.getLocalBounds().width/2, this->sprite_.getLocalBounds().height/2);
      float scale = GetScale(this->sprite_.getGlobalBounds().height, this->player_height_);
      this->sprite_.setScale(scale, scale); 
      this->sprite_.setPosition(screen_width*0.5, screen_height*0.7);
      this->sprite_.setColor(sf::Color::Red);

      this->bullet_texture_ = bullet_texture;
      // Calc the maximum amount of bullets based on screen size, bullet speed and fire cooldown
      //this->bullets_.reserve((screen_height/Bullet::speed_)/this->fire_cooldown);
      
    }

    void Update(sf::RenderWindow &window, int elapsed){
      // Lock minimum value to 0
      this->remaining_fire_cooldown = (this->remaining_fire_cooldown < 0)? 0 : this->remaining_fire_cooldown - elapsed; 
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
          this->magazine_.AddBullet(true, this->sprite_.getPosition().x, this->sprite_.getGlobalBounds().top);
          this->remaining_fire_cooldown = this->fire_cooldown;
        }
      }

      this->magazine_.Update(window, elapsed); 
      window.draw(this->sprite_);
    }

    void ClearBullets(){
      this->magazine_.ClearBullets();
    }
    
    AlienAccessPlayerMagazine* GetPlayerMagazine(){
      //AlienAccessMagazine *player_magazine = &this->magazine_; 
      return this->magazine_.GetPlayerMagazine();
    }
    
};

class Alien: protected Movable{
  private: 
    sf::Sprite sprite_;
    //const int alien_width_ = 100;        // Pixels
    const float speed_ = 0.3125;           // Pixels / millisecond
    int soldier_num;
    sf::FloatRect hitbox_;
    const float hitbox_x_margin_percentage_ = 0.25;
    const float hitbox_y_margin_percentage_ = 0.35;
    bool active_ = true;
  public:
    static const int alien_width_;
    Alien(sf::Texture *alien_texture, int soldier_num, int formation_col, int formation_row, int screen_margin, int row_margin, int col_margin){
      this->sprite_.setTexture(*alien_texture);
      this->sprite_.setOrigin(this->sprite_.getLocalBounds().width/2, this->sprite_.getLocalBounds().height/2);
      float scale = GetScale(this->sprite_.getGlobalBounds().width, this->alien_width_);
      this->sprite_.setScale(scale, scale);
     
      // Set alien at initial position (checking screen margin) 
      this->sprite_.setPosition(this->sprite_.getGlobalBounds().width + screen_margin, this->sprite_.getGlobalBounds().height);
      // Move to relative position based on row/col margins and formation position
      this->sprite_.move(
          (
            this->sprite_.getGlobalBounds().width
            + row_margin
            + screen_margin
          )  * formation_col,
          (
            this->sprite_.getGlobalBounds().height
            + col_margin
          )  * formation_row
      );
      this->sprite_.setColor(sf::Color::Red);

      // Create hitbox
      float sprite_width = this->sprite_.getLocalBounds().width;
      float hitbox_x_margin = sprite_width * hitbox_x_margin_percentage_;
      float hitbox_width = sprite_width - (hitbox_x_margin * 2); 

      float sprite_height = this->sprite_.getLocalBounds().height;
      float hitbox_y_margin = sprite_height * hitbox_y_margin_percentage_;
      float hitbox_height = sprite_height - (hitbox_y_margin * 2);
      this->hitbox_ = {hitbox_x_margin, hitbox_y_margin, hitbox_width, hitbox_height};
    }

    void Move(Movable::Direction direction, int elapsed){
      Movable::Move(this->sprite_, direction, elapsed * this->speed_);
    }
    void Draw(sf::RenderWindow &window){
      window.draw(this->sprite_);
    }

    sf::FloatRect GetHitBox(){
      return this->sprite_.getTransform().transformRect(this->hitbox_);
    }

    void Dead(){
      this->active_ = false;
    }
};
constexpr int Alien::alien_width_ = 100;

class AlienCovenant: protected Movable{
  private:
    std::vector<Alien*> covenant_;
    Movable::Direction movement_loop[8];  
    int current_direction; 
    const int stall_duration = 1200;  // Milliseconds
    const int movement_duration = 200;        // Milliseconds
    float elapsed_stall = 0;
    float elapsed_movement = 0;
    const int row_margin_ = 15;  // pixels
    const int col_margin_ = 15;
    const int screen_margin_ = 10;
    int aliens_per_row_;
  protected:
    Movable::Direction GetCurrentDirection(){
      return movement_loop[this->current_direction];
    }
    void UpdateDirection(){
      this->current_direction = (this->current_direction+1) % 8;    // todo NOT hardcoded
    }
  public:
    AlienCovenant(char num_aliens, sf::Texture *alien_texture, const unsigned int &screen_width){
      this->movement_loop[0] = Movable::Direction::kRight;
      this->movement_loop[1] = Movable::Direction::kDown;
      this->movement_loop[2] = Movable::Direction::kLeft;
      this->movement_loop[3] = Movable::Direction::kUp;
      this->movement_loop[4] = Movable::Direction::kRight;
      this->movement_loop[5] = Movable::Direction::kDown;
      this->movement_loop[6] = Movable::Direction::kLeft;
      this->movement_loop[7] = Movable::Direction::kDown;

      this->current_direction = 0;

      this->covenant_.reserve(num_aliens);

      // Create aliens
      this->aliens_per_row_ = screen_width/(Alien::alien_width_ + this->row_margin_ + (2 * this->screen_margin_));
      for(int i = 0; i<num_aliens; ++i){
        int formation_col = i%this->aliens_per_row_;
        int formation_row = i/this->aliens_per_row_;
        this->covenant_.push_back(
          new Alien(alien_texture, i, formation_col, formation_row, this->screen_margin_, this->row_margin_, this->col_margin_)
        );
      }
    }
    ~AlienCovenant(){
      for(Alien *alien : this->covenant_){
        delete alien;
        alien = nullptr;
      }
    }

    void Update(sf::RenderWindow &window, int elapsed, AlienAccessPlayerMagazine *player_magazine){
      elapsed_stall += elapsed;
      if(elapsed_stall > stall_duration){
        elapsed_movement += elapsed;

        if(elapsed_movement > movement_duration){
          elapsed_stall = 0;
          elapsed_movement = 0;
          this->UpdateDirection();
        }
      }
      for(Alien *soldier : this->covenant_){
        // movement here check to avoid double covenant range-base loop
        if(elapsed_movement > 0 && elapsed_movement <= movement_duration){
          soldier->Move(AlienCovenant::GetCurrentDirection(), elapsed);
        }
        unsigned int i = 0;
        for(sf::FloatRect bullet_hitbox : (*player_magazine).GetBulletsHitBoxes()){
          if(soldier->GetHitBox().intersects(bullet_hitbox)){
            std::cout << "PWND\n";
            soldier->Dead();
            (*player_magazine).DeleteBullet(i);
          }
          ++i;
        }
        /* Debug
        sf::RectangleShape rectangle;
        rectangle.setPosition(soldier->GetHitBox().left, soldier->GetHitBox().top);
        rectangle.setSize(sf::Vector2f(soldier->GetHitBox().width, soldier->GetHitBox().height));
        rectangle.setFillColor(sf::Color::Red);
        window.draw(rectangle);
        */
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
  AlienCovenant covenant = AlienCovenant(1, textures->GetAlien(), screen_width);
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
      //AlienAccessPlayerMagazine *player_magazine = player.GetPlayerMagazine();
      //player_magazine->GetBulletsHitBoxes();
      covenant.Update(window, elapsed.asMilliseconds(), player.GetPlayerMagazine());

      window.display();
    }else{
      // todo check if performant and good practice
      // Do performance tasks
      player.ClearBullets();
    }
  }
  
  return 0;
}
