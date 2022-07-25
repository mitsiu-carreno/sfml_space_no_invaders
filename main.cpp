// g++ main.cpp -o main -I SFML-2.5.1/include/ -L SFML-2.5.1/lib/ -lsfml-graphics -lsfml-window -lsfml-system  && export LD_LIBRARY_PATH=SFML-2.5.1/lib/  && ./main
#include <SFML/Graphics.hpp>
#include <iostream>
#include <algorithm>    // std::remove_if
#include <vector>
#include <stdlib.h>     // srand rand
#include <time.h>       // time
#include <math.h>       // log
// todo use window.getSize() ??
// todo keypress on class?
// covenant movement on function ptr?
// covenant formation on function ptr?



float GetScale(float initial_measure, float target_measure){
  return target_measure/initial_measure;
}

int GetPseudoRandom(int min, int max){
  // Really basic no need for overcomplicated solutions 
  // just want to pseudo random fire from aliens
  return std::rand() % max;
}

namespace constants{
  constexpr int kFrameRate = 60;  // Frames per second
  constexpr int kFrameDuration = 1000/kFrameRate;
  constexpr int kScreenMargins = 10;
  constexpr int kMaxHitboxPerSprite = 3;
}

class Texture{
  private:
    sf::Texture player_;
    sf::Texture alien_;
    sf::Texture rocket_;
    sf::Texture laser_;
  public:
    Texture(){
      // Codes: Portal, spiderman/ironman, game mechanic elites desapear on stall
      if(!this->player_.loadFromFile("resources/plane.png")){
        throw "Unable to load ship texture";
      }
      if(!this->alien_.loadFromFile("resources/white-alien.png")){
        throw "Unable to load alien texture";
      }
      if(!this->rocket_.loadFromFile("resources/rocket.png")){
        throw "Unable to load rocket texture";
      }
      if(!this->laser_.loadFromFile("resources/laser.png")){
        throw "Unable to load laser texture";
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
    sf::Texture* GetLaser(){
      return &this->laser_;
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

class Hitboxable{
  protected:
    std::vector<sf::FloatRect> hitboxes_;
    std::vector<sf::FloatRect> hitboxes_transformed_;
    Hitboxable(){
      this->hitboxes_.reserve(constants::kMaxHitboxPerSprite);
      this->hitboxes_transformed_.reserve(constants::kMaxHitboxPerSprite);
    }
    void SetHitbox(
      const sf::Sprite &sprite,
      float percentage_offset_x,
      float percentage_offset_y,
      float percentage_x,
      float percentage_y
    ){
      float sprite_width = sprite.getLocalBounds().width;
      float hitbox_width = sprite_width * percentage_x;
      float hitbox_offset_x = sprite_width * percentage_offset_x;

      float sprite_height = sprite.getLocalBounds().height;
      float hitbox_height = sprite_height * percentage_y;
      float hitbox_offset_y = sprite_width * percentage_offset_y;
      this->hitboxes_.push_back(
          {hitbox_offset_x, hitbox_offset_y, hitbox_width, hitbox_height}
      );
      this->hitboxes_transformed_.resize(this->hitboxes_.size());
    }

    std::vector<sf::FloatRect>* GetHitboxes(const sf::Sprite &sprite){
      for(int i=0; i < this->hitboxes_.size(); ++i){
        this->hitboxes_transformed_.at(i) = sprite.getTransform().transformRect(this->hitboxes_.at(i));
      }
      return &this->hitboxes_transformed_;
    }

    bool CheckCollision(const sf::Sprite &sprite, const sf::FloatRect &enemy_hitbox){
      for(int i=0; i < this->hitboxes_.size(); ++i){
        if(
          sprite.getTransform().
            transformRect(this->hitboxes_.at(i))
            .intersects(enemy_hitbox)
        ){
          return true;
        }
      }
      return false;
    }
};

class Projectile: protected Movable, protected Hitboxable{
  private:
    sf::Sprite sprite_;
    bool active_ = true;
  public:
    static constexpr float speed_ = 0.6;
    static constexpr int projectile_height_ = 40; // pixels /milliseconds
    Projectile(sf::Texture& texture, float pos_x, float pos_y){
      this->sprite_.setTexture(texture);
      this->sprite_.setOrigin(this->sprite_.getLocalBounds().width/2, this->sprite_.getLocalBounds().height/2);
      float scale = GetScale(this->sprite_.getGlobalBounds().height, this->projectile_height_);
      this->sprite_.setScale(scale, scale);
      this->sprite_.setPosition(pos_x, pos_y);
      this->SetHitbox(this->sprite_, 0, 0, 1, 1);
    }
    void Update(sf::RenderWindow &window, int elapsed, bool friendly){
      Movable::Direction direction;
      if(friendly){
        direction = Movable::Direction::kUp;
      }else{
        direction = Movable::Direction::kDown;
      }
      Movable::Move(this->sprite_, direction, elapsed * this->speed_);
      window.draw(this->sprite_);
    }
    sf::Vector2f GetPosition() const {
      return this->sprite_.getPosition();
    }

    std::vector<sf::FloatRect>* GetHitboxes(){
      return Hitboxable::GetHitboxes(this->sprite_);
    }
    bool GetActiveStatus() const {
      return this->active_;
    }
};

class PublicAccessMagazine{  
  private: 
    std::vector<Projectile>* projectiles_ptr_;
  public:
    void SetProjectilesPointer(std::vector<Projectile> *p){
      this->projectiles_ptr_ = p;
    }
    std::vector<sf::FloatRect> GetProjectilesHitBoxes(){
      std::vector<sf::FloatRect> temp;
      temp.reserve((*this->projectiles_ptr_).size() * constants::kMaxHitboxPerSprite);  // reserve enough space for worst case scenario (try to avoid moving in memory)
      for(Projectile &projectile : (*this->projectiles_ptr_)){
        for(const sf::FloatRect &hitbox : *projectile.GetHitboxes()){
          temp.push_back(hitbox);
        }
      }
      return temp;
    }

    void DeleteProjectile(unsigned int i){
      (*this->projectiles_ptr_).erase((*this->projectiles_ptr_).begin() + i);
    }
};

class ProjectileMagazine{
  private:
    sf::Texture *projectile_texture_;
    std::vector<Projectile> projectiles_;
    PublicAccessMagazine public_magazine_;
    bool friendly_;
    unsigned int screen_height;
    //int max_size=0;
  public:
    ProjectileMagazine(bool friendly, sf::Texture *projectile_texture, int max_projectiles, unsigned int screen_height){
      this->projectile_texture_ = projectile_texture;
      this->friendly_ = friendly;
      this->screen_height = screen_height;

      this->projectiles_.reserve(max_projectiles); 
      public_magazine_.SetProjectilesPointer(&this->projectiles_);
    }
    void Update(sf::RenderWindow &window, int elapsed){
      for(Projectile &projectile : this->projectiles_){
        projectile.Update(window, elapsed, this->friendly_);
        /* Debug 
        for(const sf::FloatRect &hitbox : *projectile.GetHitboxes()){
          
          sf::RectangleShape rectangle;
          rectangle.setPosition(hitbox.left, hitbox.top);
          rectangle.setSize(sf::Vector2f(hitbox.width, hitbox.height));
          rectangle.setFillColor(sf::Color::Transparent);
          rectangle.setOutlineColor(sf::Color::Red);
          rectangle.setOutlineThickness(3.f);

          window.draw(rectangle);
        }
        */
      }
    }
    void AddProjectile(float pos_x, float pos_y){
      Projectile *new_projectile = new Projectile(
          *(this->projectile_texture_),
          pos_x,
          pos_y
      );
      this->projectiles_.push_back(*new_projectile);
      delete new_projectile;
    }
    void ClearProjectiles(){  
      // DEBUG
      //std::cout << "cap" << this->projectiles_.capacity() << " size:" << this->projectiles_.size() << "\n";
      /*  DEBUG
          if(this->projectiles_.capacity() > this->max_size){
            this->max_size = this->projectiles_.capacity();
          }
          std::cout << this->max_size << "\n";
          */
      /*  LEGACY
      unsigned i = 0;
      unsigned deleted = -1;
      for(Projectile &projectile : this->projectiles_){
        if(projectile.GetPosition().y < 0 - Projectile::projectile_height_ || !projectile.GetActiveStatus()){
          //std::cout << "Will delete at " << projectile.GetPosition().y << "\n";
          std::cout << "cap" << this->projectiles_.capacity() << " size:" << this->projectiles_.size() << "\n";
          deleted = i;
        }
        ++i;
      }
      if(deleted != -1){
        this->projectiles_.erase(this->projectiles_.begin()+deleted);
      }
      */
      auto end = std::remove_if(this->projectiles_.begin(), this->projectiles_.end(),
          [this](const Projectile &projectile){
            return (
                projectile.GetPosition().y < 0 - Projectile::projectile_height_ 
                || !projectile.GetActiveStatus()
                || projectile.GetPosition().y > this->screen_height
            );
          }
      );
      this->projectiles_.erase(end, this->projectiles_.end());
    }
    unsigned int GetMagazineSize(){
      return this->projectiles_.size();
    }
    PublicAccessMagazine* GetMagazine(){
      return &(this->public_magazine_);
    }
};

class Player: protected Movable, protected Hitboxable{    // todo Inherit from sprite?
  private:
    sf::Sprite sprite_;
    static constexpr int player_height_ = 150; // Pixels
    static constexpr float speed_ = 0.625;      // pixels / millisecond
    static constexpr int fire_cooldown = 300;
    int remaining_fire_cooldown = fire_cooldown; 
    ProjectileMagazine bullet_magazine_;
    static constexpr float hitbox_x_margin_percentaje_ = 0.4;
  public:
    Player(
        sf::Texture *player_texture, 
        unsigned int screen_width, 
        unsigned int screen_height, 
        sf::Texture *bullet_texture
    )
      :
        bullet_magazine_(
            true, 
            bullet_texture,
            (screen_height/Projectile::speed_)/this->fire_cooldown,
            screen_height
        )
    {
      this->sprite_.setTexture(*player_texture);
      this->sprite_.setOrigin(this->sprite_.getLocalBounds().width/2, this->sprite_.getLocalBounds().height/2);
      float scale = GetScale(this->sprite_.getGlobalBounds().height, this->player_height_);
      this->sprite_.setScale(scale, scale); 
      this->sprite_.setPosition(screen_width*0.5, screen_height*0.7);

      this->SetHitbox(this->sprite_, .4, .1, .2, .9);
      this->SetHitbox(this->sprite_, .185, .50, .63, .4);
      this->SetHitbox(this->sprite_, 0, .52, 1, .27);
    }

    void Update(sf::RenderWindow &window, int elapsed, bool &playing, PublicAccessMagazine *covenant_magazine){
      // Lock minimum value to 0
      this->remaining_fire_cooldown = (this->remaining_fire_cooldown < 0)? 0 : this->remaining_fire_cooldown - elapsed; 
      if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left)){
        if(this->sprite_.getGlobalBounds().left > constants::kScreenMargins){
          Movable::Move(this->sprite_, Direction::kLeft, elapsed * this->speed_);
        }
      }
      if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right)){
        sf::Vector2f point = {
          this->sprite_.getGlobalBounds().left + this->sprite_.getGlobalBounds().width,
          this->sprite_.getGlobalBounds().top + this->sprite_.getGlobalBounds().height
        };
        if(window.mapCoordsToPixel(point).x < window.getSize().x - constants::kScreenMargins){
          Movable::Move(this->sprite_, Movable::Direction::kRight, elapsed * this->speed_);
        }
      }
      if(sf::Keyboard::isKeyPressed(sf::Keyboard::Space)){
        if(this->remaining_fire_cooldown == 0){
          this->bullet_magazine_.AddProjectile(this->sprite_.getPosition().x, this->sprite_.getGlobalBounds().top);
          this->remaining_fire_cooldown = this->fire_cooldown;
        }
      }

      this->bullet_magazine_.Update(window, elapsed); 
      for(const sf::FloatRect &laser_hitbox : (*covenant_magazine).GetProjectilesHitBoxes()){
        if(this->CheckCollision(laser_hitbox)){
          std::cout << "PWND dead\n";
          playing = false;
        }
        
      }

      /*
      for(const sf::FloatRect &hitbox : *(this->GetHitboxes(this->sprite_))){
        sf::RectangleShape rectangle;
        rectangle.setPosition(hitbox.left, hitbox.top);
        rectangle.setSize(sf::Vector2f(hitbox.width, hitbox.height));
        rectangle.setFillColor(sf::Color::Blue);
        window.draw(rectangle);
      }
      */

      window.draw(this->sprite_);
    }
    /*
    std::vector<sf::FloatRect>* GetHitboxes(){
      return Hitboxable::GetHitboxes(this->sprite_);
      //return this->sprite_.getTransform().transformRect(this->hitbox_);
    }
    */
    bool CheckCollision(const sf::FloatRect &enemy_hitbox){
      return Hitboxable::CheckCollision(this->sprite_, enemy_hitbox);
    }

    void ClearProjectiles(){
      this->bullet_magazine_.ClearProjectiles();
    }
    
    PublicAccessMagazine* GetPlayerMagazine(){
      return this->bullet_magazine_.GetMagazine();
    }

    std::vector<sf::FloatRect>* GetHitboxes(){
      return Hitboxable::GetHitboxes(this->sprite_);
    }
    
};

class Alien: protected Movable, protected Hitboxable{
  private: 
    sf::Sprite sprite_;
    //const int alien_width_ = 100;        // Pixels
    static constexpr float speed_ = 0.3129;           // Pixels / millisecond
    char soldier_num_;
    static constexpr float hitbox_x_margin_percentage_ = 0.25;
    static constexpr float hitbox_y_margin_percentage_ = 0.35;
    bool active_ = true;
    int fire_cooldown_;
    static constexpr int min_fire_cooldown_ = 7000;
    static constexpr int max_fire_cooldown_ = 12000;
  public:
    static constexpr int alien_width_ = 100;
    Alien(sf::Texture *alien_texture, char soldier_num, float pos_x, float pos_y){
      this->soldier_num_ = soldier_num;
      this->sprite_.setTexture(*alien_texture);
      this->sprite_.setOrigin(this->sprite_.getLocalBounds().width/2, this->sprite_.getLocalBounds().height/2);
      float scale = GetScale(this->sprite_.getGlobalBounds().width, this->alien_width_);
      this->sprite_.setScale(scale, scale);
   
      // Pos received are top-left, must adecuate to actual origin (center-center)
      this->sprite_.setPosition(
        pos_x + this->sprite_.getGlobalBounds().width/2, 
        pos_y + this->sprite_.getGlobalBounds().height/2
      ); 

      this->sprite_.setColor(sf::Color::Red);
      this->fire_cooldown_ = GetPseudoRandom(this->min_fire_cooldown_, this->max_fire_cooldown_);

      this->SetHitbox(
          this->sprite_, 
          this->hitbox_x_margin_percentage_, 
          this->hitbox_y_margin_percentage_, 
          1-(this->hitbox_x_margin_percentage_*2), 
          1-(this->hitbox_y_margin_percentage_*2)
      );
    }

    void Move(Movable::Direction direction, int elapsed){
      Movable::Move(this->sprite_, direction, elapsed * this->speed_);
    }
    void Draw(sf::RenderWindow &window){
      window.draw(this->sprite_);
    }


    bool GetActiveStatus() const {
      return this->active_;
    }
    void Dead(){
      this->active_ = false;
    }

    bool UpdateFireCooldown(int elapsed){
      this->fire_cooldown_ -= elapsed;
      if(this->fire_cooldown_ < 0){
        this->fire_cooldown_ = GetPseudoRandom(this->min_fire_cooldown_, this->max_fire_cooldown_);
        return true;
      }
      return false;
    }

    sf::Vector2f GetPosition(){
      return this->sprite_.getPosition();
    }
    bool CheckCollision(const sf::FloatRect &enemy_hitbox){
      return Hitboxable::CheckCollision(this->sprite_, enemy_hitbox);
    }
    std::vector<sf::FloatRect>* GetHitboxes(){
      return Hitboxable::GetHitboxes(this->sprite_);
    }
};

void StandardFormation(const char soldier_number, const char total_soldiers, const unsigned int &screen_width, float &pos_x, float &pos_y){
  constexpr int screen_margin = 10;
  constexpr int row_margin = 15;
  constexpr int col_margin = 15;
  const int aliens_per_row = screen_width/(Alien::alien_width_ + row_margin + (2 * screen_margin));

  int formation_col = soldier_number%aliens_per_row;
  int formation_row = soldier_number/aliens_per_row;
  pos_x = screen_margin + ((Alien::alien_width_ + row_margin) * formation_col);
  pos_y = screen_margin + ((Alien::alien_width_ + col_margin) * formation_row);
}

void LogFormation(const char soldier_number, const char total_soldiers, const unsigned int &screen_width, float &pos_x, float &pos_y){
  constexpr int screen_margin = 10;
  //constexpr int row_margin = 15;
  const int margin = (screen_width - screen_margin * 2)/total_soldiers;
  //first soldier_num is 0 log = inf
  int x = (1 + soldier_number) * margin;
  pos_x = x;
  pos_y = (2 * log10 (2*x) ); 
  //pos_y = log10(x);
  std::cout << "x: " << pos_x << " y: " << pos_y << "\n";
}

/*
void CresendoWaveFormation(){
  sin(x) * log(x)
}
*/

class AlienCovenant: protected Movable{
  private:
    std::vector<Alien> covenant_;
    Movable::Direction movement_loop[8];  
    int current_direction; 
    static constexpr int stall_duration = 3000;  // Milliseconds
    static constexpr int movement_duration = 200;        // Milliseconds
    float elapsed_stall = 0;
    float elapsed_movement = 0;
    static constexpr int max_lasers_per_frame_ = 20;
    int aliens_per_row_;
    ProjectileMagazine laser_magazine_;
  protected:
    Movable::Direction GetCurrentDirection(){
      return movement_loop[this->current_direction];
    }
    void UpdateDirection(){
      this->current_direction = (this->current_direction+1) % 8;    // todo NOT hardcoded
    }
  public:
    AlienCovenant(
        char num_aliens, 
        sf::Texture *alien_texture, 
        const unsigned int &screen_width, 
        const unsigned int &screen_height, 
        sf::Texture *laser_texture
      ):laser_magazine_(
        false, 
        laser_texture,
        this->max_lasers_per_frame_,
        screen_height
    ){
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

      //void (*GetFormationPtr)(const char, const char, const unsigned int&,  float&, float&) = &StandardFormation;
      void (*GetFormationPtr)(const char, const char, const unsigned int&,  float&, float&) = &LogFormation;
      // Create aliens
      for(char i=0; i < num_aliens; ++i){
        float pos_x;
        float pos_y;
        (*GetFormationPtr)(i, num_aliens, screen_width, pos_x, pos_y);
        Alien *new_alien = new Alien(alien_texture, i, pos_x, pos_y);
        this->covenant_.push_back(*new_alien);
        delete new_alien;
      }

    }
    /*
    ~AlienCovenant(){
      for(Alien *alien : this->covenant_){
        delete alien;
        alien = nullptr;
      }
    }
    */

    void Update(sf::RenderWindow &window, int elapsed, bool &playing, bool &win, PublicAccessMagazine *player_magazine, std::vector<sf::FloatRect>* player_hitbox){
      //std::cout << "cap" << this->covenant_.capacity() << " size:" << this->covenant_.size() << "\n";
      elapsed_stall += elapsed;
      if(elapsed_stall > stall_duration){
        elapsed_movement += elapsed;

        if(elapsed_movement > movement_duration){
          elapsed_stall = 0;
          elapsed_movement = 0;
          this->UpdateDirection();
        }
      }
      for(Alien &soldier : this->covenant_){
        if(soldier.UpdateFireCooldown(elapsed) && this->laser_magazine_.GetMagazineSize() < this->max_lasers_per_frame_){
          sf::Vector2f coords = soldier.GetPosition();
          this->laser_magazine_.AddProjectile(coords.x, coords.y);
        }
        // movement here check to avoid double covenant range-base loop
        if(elapsed_movement > 0 && elapsed_movement <= movement_duration){
          soldier.Move(AlienCovenant::GetCurrentDirection(), elapsed);
          if(soldier.GetPosition().y > window.getSize().y){
            playing = false;
          }
        }
        // Check collision with bullets or player
        unsigned int i = 0;
        for(const sf::FloatRect &bullet_hitbox : (*player_magazine).GetProjectilesHitBoxes()){
          if(soldier.CheckCollision(bullet_hitbox)){
            std::cout << "PWND\n";
            soldier.Dead();
            (*player_magazine).DeleteProjectile(i);
          }
          ++i;
        }
        for(const sf::FloatRect &player_hitbox : (*player_hitbox)){
          if(soldier.CheckCollision(player_hitbox)){
            std::cout << "PWND Reached\n";
            playing = false;
          }
        }
        /* Debug
        for(const sf::FloatRect &hitbox : *soldier.GetHitboxes()){
          sf::RectangleShape rectangle;
          rectangle.setPosition(hitbox.left, hitbox.top);
          rectangle.setSize(sf::Vector2f(hitbox.width, hitbox.height));
          rectangle.setFillColor(sf::Color::Red);
          window.draw(rectangle);
        }
        */
        soldier.Draw(window);
      }
      this->laser_magazine_.Update(window, elapsed);
      if(this->covenant_.size() == 0){
        playing = false;
        win = true;
      }
    }
    void ClearAliens(){
      auto end = std::remove_if(covenant_.begin(), covenant_.end(), 
          [](const Alien &alien){return alien.GetActiveStatus() == false;}
          );  
      covenant_.erase(end, covenant_.end());
    }
    void AlienFire(){
      this->laser_magazine_.AddProjectile(100,100);
    }
    void ClearProjectiles(){
      this->laser_magazine_.ClearProjectiles();
    }

    PublicAccessMagazine* GetCovenantMagazine(){
      return this->laser_magazine_.GetMagazine();
    }
};

int main(){
  std::srand(std::time(0));
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

  sf::Font font;
  if(!font.loadFromFile("resources/KOMIKAP_.ttf")){
    std::cout << "Unable to load font" << "\n";
    return EXIT_FAILURE;
  }
  bool playing = true;
  bool win = false;

  sf::Text game_status;
  game_status.setFillColor(sf::Color::White);
  game_status.setOrigin(game_status.getLocalBounds().width/2, game_status.getLocalBounds().height/2);
  game_status.setPosition(screen_width/2, screen_height/2);
  game_status.setCharacterSize(50);
  game_status.setFont(font);

  int total_aliens_demo = 20;

  Player *player = new Player(textures->GetPlayer(), screen_width, screen_height, textures->GetRocket());
  AlienCovenant *covenant = new AlienCovenant(total_aliens_demo, textures->GetAlien(), screen_width, screen_height, textures->GetLaser());
  //////
  sf::Clock clock;
  while(window.isOpen()){
    sf::Event event;
    while(window.pollEvent(event)){
      if(event.type == sf::Event::Closed){
        std::cout << "Start Closing\n";
        delete textures;
        delete covenant;
        delete player;
        textures = nullptr;
        covenant = nullptr;
        player = nullptr;
        window.close();
        std::cout << "End Closing\n";
        return 0;
      }
      if(sf::Keyboard::isKeyPressed(sf::Keyboard::Enter) && !playing){

        delete player;
        delete covenant;
        playing = true;
        win = false;
        covenant = new AlienCovenant(total_aliens_demo, textures->GetAlien(), screen_width, screen_height, textures->GetLaser());
        player = new Player(textures->GetPlayer(), screen_width, screen_height, textures->GetRocket());
        clock.restart();
      }
    }

    if(!playing){
      window.draw(game_status);
      window.display();
      continue;
    }

    sf::Time elapsed = clock.getElapsedTime();
    if(elapsed.asMilliseconds() >= constants::kFrameDuration){
      clock.restart();
      window.clear(sf::Color(30,30,30));
   
      covenant->Update(window, elapsed.asMilliseconds(), playing, win, player->GetPlayerMagazine(), player->GetHitboxes());

      player->Update(window, elapsed.asMilliseconds(), playing, covenant->GetCovenantMagazine()); 
      if(!playing){
        if(win){
          game_status.setString("WINNER!\nEnter to continue");
          ++total_aliens_demo;
        }else{
          game_status.setString("YOU LOSER\nEnter to retry");
        }
        continue;
      }

      window.display();
    }else{
      // todo check if performant and good practice
      // Do performance tasks
      player->ClearProjectiles();
      covenant->ClearAliens();
      covenant->ClearProjectiles();
    }
  }
  
  return 0;
}
