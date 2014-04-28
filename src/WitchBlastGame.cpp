#include "WitchBlastGame.h"
#include "sfml_game/SpriteEntity.h"
#include "sfml_game/TileMapEntity.h"
#include "DungeonMap.h"
#include "sfml_game/ImageManager.h"
#include "sfml_game/SoundManager.h"
#include "sfml_game/EntityManager.h"
#include "Constants.h"
#include "RatEntity.h"
#include "GreenRatEntity.h"
#include "KingRatEntity.h"
#include "BatEntity.h"
#include "SlimeEntity.h"
#include "ChestEntity.h"
#include "EvilFlowerEntity.h"
#include "ItemEntity.h"
#include "ArtefactDescriptionEntity.h"
#include "StaticTextEntity.h"
#include "PnjEntity.h"
#include "TextEntity.h"

#include <iostream>
#include <sstream>
#include <fstream>

WitchBlastGame::WitchBlastGame(): Game(SCREEN_WIDTH, SCREEN_HEIGHT)
{
  app->setTitle(APP_NAME + " V" + APP_VERSION);

  // loading resources
  ImageManager::getImageManager()->addImage((char*)"media/sprite.png");
  ImageManager::getImageManager()->addImage((char*)"media/bolt.png");
  ImageManager::getImageManager()->addImage((char*)"media/tiles.png");
  ImageManager::getImageManager()->addImage((char*)"media/rat.png");
  ImageManager::getImageManager()->addImage((char*)"media/minimap.png");
  ImageManager::getImageManager()->addImage((char*)"media/doors.png");
  ImageManager::getImageManager()->addImage((char*)"media/items.png");
  ImageManager::getImageManager()->addImage((char*)"media/items_equip.png");
  ImageManager::getImageManager()->addImage((char*)"media/chest.png");
  ImageManager::getImageManager()->addImage((char*)"media/bat.png");
  ImageManager::getImageManager()->addImage((char*)"media/evil_flower.png");
  ImageManager::getImageManager()->addImage((char*)"media/slime.png");
  ImageManager::getImageManager()->addImage((char*)"media/king_rat.png");
  ImageManager::getImageManager()->addImage((char*)"media/blood.png");
  ImageManager::getImageManager()->addImage((char*)"media/corpses.png");
  ImageManager::getImageManager()->addImage((char*)"media/corpses_big.png");
  ImageManager::getImageManager()->addImage((char*)"media/star.png");
  ImageManager::getImageManager()->addImage((char*)"media/star2.png");
  ImageManager::getImageManager()->addImage((char*)"media/interface.png");
  ImageManager::getImageManager()->addImage((char*)"media/pnj.png");
  ImageManager::getImageManager()->addImage((char*)"media/fairy.png");

  SoundManager::getSoundManager()->addSound((char*)"media/sound/step.ogg");
  SoundManager::getSoundManager()->addSound((char*)"media/sound/blast00.ogg");
  SoundManager::getSoundManager()->addSound((char*)"media/sound/blast01.ogg");
  SoundManager::getSoundManager()->addSound((char*)"media/sound/door_closing.ogg");
  SoundManager::getSoundManager()->addSound((char*)"media/sound/door_opening.ogg");
  SoundManager::getSoundManager()->addSound((char*)"media/sound/chest_opening.ogg");
  SoundManager::getSoundManager()->addSound((char*)"media/sound/impact.ogg");
  SoundManager::getSoundManager()->addSound((char*)"media/sound/bonus.ogg");
  SoundManager::getSoundManager()->addSound((char*)"media/sound/drink.ogg");
  SoundManager::getSoundManager()->addSound((char*)"media/sound/player_hit.ogg");
  SoundManager::getSoundManager()->addSound((char*)"media/sound/player_die.ogg");
  SoundManager::getSoundManager()->addSound((char*)"media/sound/ennemy_dying.ogg");
  SoundManager::getSoundManager()->addSound((char*)"media/sound/coin.ogg");
  SoundManager::getSoundManager()->addSound((char*)"media/sound/pay.ogg");
  SoundManager::getSoundManager()->addSound((char*)"media/sound/wall_impact.ogg");
  SoundManager::getSoundManager()->addSound((char*)"media/sound/big_wall_impact.ogg");
  SoundManager::getSoundManager()->addSound((char*)"media/sound/king_rat_cry_1.ogg");
  SoundManager::getSoundManager()->addSound((char*)"media/sound/king_rat_cry_2.ogg");
  SoundManager::getSoundManager()->addSound((char*)"media/sound/king_rat_die.ogg");
  SoundManager::getSoundManager()->addSound((char*)"media/sound/slime_jump.ogg");
  SoundManager::getSoundManager()->addSound((char*)"media/sound/slime_impact.ogg");
  SoundManager::getSoundManager()->addSound((char*)"media/sound/slime_impact_weak.ogg");

  if (font.loadFromFile("media/DejaVuSans-Bold.ttf"))
  {
    myText.setFont(font);
  }

  miniMap = NULL;
  currentMap = NULL;
  currentFloor = NULL;
  specialState = SpecialStateNone;
  isPausing = false;
}

WitchBlastGame::~WitchBlastGame()
{
  // cleaning all entities
  EntityManager::getEntityManager()->clean();

  // cleaning data
  if (miniMap != NULL) delete (miniMap);
  if (currentFloor != NULL) delete (currentFloor);
}

DungeonMap* WitchBlastGame::getCurrentMap()
{
  return currentMap;
}

PlayerEntity* WitchBlastGame::getPlayer()
{
  return player;
}

void WitchBlastGame::onUpdate()
{
  float delta = getAbsolutTime() - lastTime;
  lastTime = getAbsolutTime();

  if (!isPausing)
  {

    EntityManager::getEntityManager()->animate(delta);

    if (specialState != SpecialStateNone)
    {
      timer -= delta;
      if (timer <= 0.0f)
      {
        if (specialState == SpecialStateFadeOut)
          startNewGame(false);
        else
          specialState = SpecialStateNone;
      }
    }

    if (isPlayerAlive)
    {
      if (player->getHp() <= 0)
      {
        isPlayerAlive = false;
        playMusic(MusicEnding);
      }
    }
  }
}

void WitchBlastGame::startNewGame(bool fromSaveFile)
{
  gameState = gameStateInit;

  // cleaning all entities
  EntityManager::getEntityManager()->clean();

  // cleaning data
  if (miniMap != NULL) delete (miniMap);
  if (currentFloor != NULL) delete (currentFloor);

  if (fromSaveFile)
  {
    if (!loadGame()) fromSaveFile = false;
  }
  if (!fromSaveFile)
  {
    currentFloor = new GameFloor(1);
    floorX = FLOOR_WIDTH / 2;
    floorY = FLOOR_HEIGHT / 2;

    // the player
    player = new PlayerEntity(ImageManager::getImageManager()->getImage(0),
                              this,
                              OFFSET_X + (TILE_WIDTH * MAP_WIDTH * 0.5f),
                              OFFSET_Y + (TILE_HEIGHT * MAP_HEIGHT * 0.5f));

    // the boss room is closed
    bossRoomOpened = false;
  }

  // current map (tiles)
  currentTileMap = new TileMapEntity(ImageManager::getImageManager()->getImage(IMAGE_TILES), currentMap, 64, 64, 10);
  currentTileMap->setX(OFFSET_X);
  currentTileMap->setY(OFFSET_Y);

  miniMap = new GameMap(FLOOR_WIDTH, FLOOR_HEIGHT);
  refreshMinimap();

  // the interface
  SpriteEntity* interface = new SpriteEntity(ImageManager::getImageManager()->getImage(IMAGE_INTERFACE));
  interface->setZ(10000.0f);
  interface->removeCenter();
  interface->setType(0);

  // key symbol on the interface
  keySprite.setTexture(*ImageManager::getImageManager()->getImage(IMAGE_ITEMS_EQUIP));
  keySprite.setTextureRect(sf::IntRect(ITEM_WIDTH * EQUIP_BOSS_KEY, 0,  ITEM_WIDTH, ITEM_HEIGHT));
  keySprite.setPosition(326, 616);

  // minimap on the interface
  TileMapEntity* miniMapEntity = new TileMapEntity(ImageManager::getImageManager()->getImage(4), miniMap, 16, 12, 10);
  miniMapEntity->setX(400);
  miniMapEntity->setY(607);
  miniMapEntity->setZ(10001.0f);

  // doors
  doorEntity[0] = new DoorEntity(8);
  doorEntity[1] = new DoorEntity(4);
  doorEntity[2] = new DoorEntity(2);
  doorEntity[3] = new DoorEntity(6);

  isPlayerAlive = true;

  // generate the map
  refreshMap();
  // items from save
  currentMap->restoreMapObjects(this);

  // first map is open
  roomClosed = false;

  // game time counter an state
  lastTime = getAbsolutTime();
  gameState = gameStatePlaying;

  playMusic(MusicDonjon);

  // fade in
  specialState = SpecialStateFadeIn;
  timer = FADE_IN_DELAY;

  float x0 = OFFSET_X + MAP_WIDTH * 0.5f * TILE_WIDTH;
  float y0 = OFFSET_Y + MAP_HEIGHT * 0.5f * TILE_HEIGHT + 40.0f;

  TextEntity* text = new TextEntity("Level 1", 30, x0, y0);
  text->setAlignment(ALIGN_CENTER);
  text->setLifetime(2.5f);
  text->setWeight(-36.0f);
  text->setZ(1000);
  text->setColor(TextEntity::COLOR_FADING_WHITE);
}

void WitchBlastGame::startGame()
{
    startNewGame(true);

    // Start game loop
    while (app->isOpen())
    {
        // Process events
        sf::Event event;

        while (app->pollEvent(event))
        {
            // Close window : exit
            if (event.type == sf::Event::Closed)
            {
              if (gameState == gameStatePlaying && !player->isDead() && currentMap->isCleared()) saveGame();
              app->close();
            }

            if (event.type == sf::Event::KeyPressed)
            {
              if (event.key.code == sf::Keyboard::Escape)
              {
                if (gameState == gameStatePlaying && !isPausing) isPausing = true;
                else if (gameState == gameStatePlaying && isPausing) isPausing = false;
              }

              if (event.key.code == sf::Keyboard::X)
              {
                startNewGame(false);
              }
            }

            if (event.type == sf::Event::LostFocus)
              isPausing = true;
        }

        if (gameState == gameStatePlaying && !isPausing)
        {
          if (player->canMove()) player->setVelocity(Vector2D(0.0f, 0.0f));

          if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q) || sf::Keyboard::isKeyPressed(sf::Keyboard::A))
          {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z) || sf::Keyboard::isKeyPressed(sf::Keyboard::W))
              player->move(7);
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
              player->move(1);
            else
              player->move(4);
          }
          else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
          {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z) || sf::Keyboard::isKeyPressed(sf::Keyboard::W))
              player->move(9);
            else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
              player->move(3);
            else
              player->move(6);
          }
          else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z) || sf::Keyboard::isKeyPressed(sf::Keyboard::W))
          {
            player->move(8);
          }
          else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
          {
            player->move(2);
          }

          if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            player->fire(4);
          else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            player->fire(6);
          else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
            player->fire(8);
          else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
            player->fire(2);
          else if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
          {
            sf::Vector2i mousePosition = sf::Mouse::getPosition(*app);
            int xm = mousePosition.x - player->getX();
            int ym = mousePosition.y - player->getY();

            if (abs(xm) >= abs(ym))
            {
              if (xm > 0) player->fire(6);
              else player->fire(4);
            }
            else
            {
              if (ym > 0) player->fire(2);
              else player->fire(8);
            }
          }

          if (player->isDead() && specialState == SpecialStateNone && sf::Keyboard::isKeyPressed(sf::Keyboard::Return))
          {
            specialState = SpecialStateFadeOut;
            timer = FADE_OUT_DELAY;
          }
        }

        onUpdate();
        EntityManager::getEntityManager()->sortByZ();
        onRender();

        verifyDoorUnlocking();

        if (roomClosed)
        {
          if (getEnnemyCount() == 0)
          {
            currentMap->setCleared(true);
            openDoors();
          }
        }
    }
    quitGame();
}

void WitchBlastGame::createFloor()
{
}

void WitchBlastGame::closeDoors()
{
  if (!currentMap->isCleared())
  {
    int i;
    for(i = 0; i < MAP_WIDTH; i++)
    {
      if (currentMap->getTile(i, 0) < 4) currentMap->setTile(i, 0, MAP_DOOR);
      if (currentMap->getTile(i, MAP_HEIGHT - 1) < 4) currentMap->setTile(i, MAP_HEIGHT - 1, MAP_DOOR);
    }
    for(i = 0; i < MAP_HEIGHT; i++)
    {
      if (currentMap->getTile(0, i) < 4) currentMap->setTile(0, i, MAP_DOOR);
      if (currentMap->getTile(MAP_WIDTH - 1, i) < 4) currentMap->setTile(MAP_WIDTH - 1, i, MAP_DOOR);
    }
    roomClosed = true;
  }
}

void WitchBlastGame::openDoors()
{
    int i, j;
    for(i = 0; i < MAP_WIDTH; i++)
      for(j = 0; j < MAP_WIDTH; j++)
        if (currentMap->getTile(i, j) == MAP_DOOR) currentMap->setTile(i, j, 0);
  roomClosed = false;
  SoundManager::getSoundManager()->playSound(SOUND_DOOR_OPENING);

  if (currentMap->hasNeighbourUp() == 2 && !bossRoomOpened)
    currentMap->setTile(MAP_WIDTH/2, 0, MAP_DOOR);
  else
    doorEntity[0]->openDoor();

  if (currentMap->hasNeighbourLeft() == 2 && !bossRoomOpened)
    currentMap->setTile(0, MAP_HEIGHT / 2, MAP_DOOR);
  else
    doorEntity[1]->openDoor();

  if (currentMap->hasNeighbourDown() == 2 && !bossRoomOpened)
    currentMap->setTile(MAP_WIDTH / 2, MAP_HEIGHT - 1, MAP_DOOR);
  else
    doorEntity[2]->openDoor();

  if (currentMap->hasNeighbourRight() == 2 && !bossRoomOpened)
    currentMap->setTile(MAP_WIDTH - 1, MAP_HEIGHT / 2, MAP_DOOR);
  else
    doorEntity[3]->openDoor();
}

int WitchBlastGame::getEnnemyCount()
{
  int n=0;

  EntityManager::EntityList* entityList =EntityManager::getEntityManager()->getList();
  EntityManager::EntityList::iterator it;

	for (it = entityList->begin (); it != entityList->end ();)
	{
		GameEntity *e = *it;
		it++;

		if (e->getType() >= 20)
		{
		  n++;
		} // endif
	} // end for

	return n;
}

void WitchBlastGame::refreshMap()
{
  // clean the sprites from old map
  EntityManager::getEntityManager()->partialClean(10);

  // if new map, it has to be randomized
  bool generateMap = !(currentFloor->getMap(floorX, floorY)->isVisited());
  currentMap = currentFloor->getAndVisitMap(floorX, floorY);

  // load the map
  currentTileMap->setMap(currentMap);
  player->setMap(currentMap, TILE_WIDTH, TILE_HEIGHT, OFFSET_X, OFFSET_Y);
  refreshMinimap();

  if(generateMap)
    this->generateMap();
  else
  {
    if (currentMap->getRoomType() == roomTypeMerchant)
      new PnjEntity(OFFSET_X + (MAP_WIDTH / 2) * TILE_WIDTH + TILE_WIDTH / 2,
                  OFFSET_Y + (MAP_HEIGHT / 2 - 2) * TILE_HEIGHT,
                  0);
  }

  // for testing purpose (new stuff)
  if (player->getAge() <2.0f)
  {
    /*ItemEntity* book = new ItemEntity(ItemEntity::itemHealth, player->getX(), player->getY()- 180);
    book->setMap(currentMap, TILE_WIDTH, TILE_HEIGHT, OFFSET_X, OFFSET_Y);
    book->setMerchandise(true);*/

    /*int bonusType = getRandomEquipItem(true);
    ItemEntity* boots = new ItemEntity((enumItemType)(itemMagicianHat + bonusType), player->getX(), player->getY()+ 180);
    boots->setMap(currentMap, TILE_WIDTH, TILE_HEIGHT, OFFSET_X, OFFSET_Y);*/

    //ChestEntity* chest = new ChestEntity(player->getX() + 100, player->getY()+ 150, CHEST_FAIRY, false);
    //chest->setMap(currentMap, TILE_WIDTH, TILE_HEIGHT, OFFSET_X, OFFSET_Y);
  }

  // check doors
  doorEntity[0]->setVisible(currentMap->hasNeighbourUp() > 0);
  if (currentMap->hasNeighbourUp() == 1) doorEntity[0]->setDoorType(0);
  if (currentMap->hasNeighbourUp() == 2 || currentMap->getRoomType()==roomTypeBoss) doorEntity[0]->setDoorType(1);
  if (currentMap->hasNeighbourUp() == 2 && !bossRoomOpened)
  {
    doorEntity[0]->setOpen(false);
    currentMap->setTile(MAP_WIDTH/2, 0, MAP_DOOR);
  }
  else
    doorEntity[0]->setOpen(true);

  doorEntity[3]->setVisible(currentMap->hasNeighbourRight() > 0);
  if (currentMap->hasNeighbourRight() == 1) doorEntity[3]->setDoorType(0);
  if (currentMap->hasNeighbourRight() == 2 || currentMap->getRoomType()==roomTypeBoss) doorEntity[3]->setDoorType(1);
  if (currentMap->hasNeighbourRight() == 2 && !bossRoomOpened)
  {
    doorEntity[3]->setOpen(false);
    currentMap->setTile(MAP_WIDTH - 1, MAP_HEIGHT / 2, MAP_DOOR);
  }
  else
    doorEntity[3]->setOpen(true);

  doorEntity[2]->setVisible(currentMap->hasNeighbourDown() > 0);
  if (currentMap->hasNeighbourDown() == 1) doorEntity[2]->setDoorType(0);
  if (currentMap->hasNeighbourDown() == 2 || currentMap->getRoomType()==roomTypeBoss) doorEntity[2]->setDoorType(1);
  if (currentMap->hasNeighbourDown() == 2 && !bossRoomOpened)
  {
    doorEntity[2]->setOpen(false);
    currentMap->setTile(MAP_WIDTH/2, MAP_HEIGHT - 1, MAP_DOOR);
  }
  else
    doorEntity[2]->setOpen(true);

  doorEntity[1]->setVisible(currentMap->hasNeighbourLeft() > 0);
  if (currentMap->hasNeighbourLeft() == 1) doorEntity[1]->setDoorType(0);
  if (currentMap->hasNeighbourLeft() == 2 || currentMap->getRoomType()==roomTypeBoss) doorEntity[1]->setDoorType(1);
  if (currentMap->hasNeighbourLeft() == 2 && !bossRoomOpened)
  {
    doorEntity[1]->setOpen(false);
    currentMap->setTile(0, MAP_HEIGHT / 2, MAP_DOOR);
  }
  else
    doorEntity[1]->setOpen(true);
}

void WitchBlastGame::refreshMinimap()
{
  for (int j=0; j < FLOOR_HEIGHT; j++)
    for (int i=0; i < FLOOR_WIDTH; i++)
    {
      int n = currentFloor->getRoom(i, j);
      if (n > 0 && currentFloor->getMap(i, j)->isVisited())
      {
        if (currentFloor->getRoom(i, j) == roomTypeStarting
            || currentFloor->getRoom(i, j) == roomTypeBonus
            || currentFloor->getRoom(i, j) == roomTypeKey)
          miniMap->setTile(i, j, roomTypeStandard);
        else
          miniMap->setTile(i, j, currentFloor->getRoom(i, j));
      }
      else if (n > 0 && currentFloor->getMap(i, j)->isKnown())
      {
        if (currentFloor->getRoom(i, j) == roomTypeBoss)
          miniMap->setTile(i, j, 7);
        else
          miniMap->setTile(i, j, 9);
      }
      else
        miniMap->setTile(i, j, 0);
    }
  miniMap->setTile(floorX, floorY, 8);
}

void WitchBlastGame::checkEntering()
{
  if (!currentMap->isCleared())
  {
    player->setEntering();
    SoundManager::getSoundManager()->playSound(SOUND_DOOR_CLOSING);
    for (int i=0; i<4; i++)
      doorEntity[i]->closeDoor();
  }
}

void WitchBlastGame::saveMapItems()
{
  EntityManager::EntityList* entityList = EntityManager::getEntityManager()->getList();
  EntityManager::EntityList::iterator it;

	for (it = entityList->begin (); it != entityList->end ();)
	{
		GameEntity* e = *it;
		it++;

    ItemEntity* itemEntity = dynamic_cast<ItemEntity*>(e);
    ChestEntity* chestEntity = dynamic_cast<ChestEntity*>(e);

		if (itemEntity != NULL)
		{
      currentMap->addItem(itemEntity->getItemType(), itemEntity->getX(), itemEntity->getY(), itemEntity->getMerchandise());
		} // endif
		else if (chestEntity != NULL)
		{
      currentMap->addChest(chestEntity->getChestType(), chestEntity->getOpened(), chestEntity->getX(), chestEntity->getY());
		} // endif
		else
    {
      SpriteEntity* spriteEntity = dynamic_cast<SpriteEntity*>(e);
      if (spriteEntity != NULL && (e->getType() == ENTITY_BLOOD || e->getType() == ENTITY_CORPSE ) )
      {
        int spriteFrame = spriteEntity->getFrame();
        if (spriteEntity->getWidth() == 128) spriteFrame += FRAME_CORPSE_KING_RAT;
        currentMap->addSprite(e->getType(), spriteFrame, e->getX(), e->getY(), spriteEntity->getScaleX());
      }

    }
	} // end for
}

void WitchBlastGame::moveToOtherMap(int direction)
{
  saveMapItems();
  switch (direction)
  {
    case (4): floorX--;  player->moveTo((OFFSET_X + MAP_WIDTH * TILE_WIDTH), player->getY()); player->move(4);  break;
    case (6): floorX++;  player->moveTo(OFFSET_X, player->getY()); player->move(6); break;
    case (8): floorY--;  player->moveTo(player->getX(), OFFSET_Y + MAP_HEIGHT * TILE_HEIGHT - 10); player->move(8); break;
    case (2): floorY++;  player->moveTo(player->getX(), OFFSET_Y);  break;
  }
  refreshMap();
  checkEntering();
  currentMap->restoreMapObjects(this);
}

void WitchBlastGame::onRender()
{
    // clear the view
    app->clear(sf::Color(32, 32, 32));

    // render the game objects
    EntityManager::getEntityManager()->render(app);

    myText.setColor(sf::Color(255, 255, 255, 255));
    myText.setCharacterSize(17);
    myText.setString("WASD or ZQSD to move\nArrows to shoot");
    myText.setPosition(650, 650);
    app->draw(myText);

    myText.setCharacterSize(18);
    std::ostringstream oss;
    oss << player->getGold();
    myText.setString(oss.str());
    myText.setPosition(690, 612);
    app->draw(myText);

    myText.setColor(sf::Color(0, 0, 0, 255));
    myText.setCharacterSize(16);
    myText.setString("Level 1");
    myText.setPosition(410, 692);
    app->draw(myText);

    sf::RectangleShape rectangle(sf::Vector2f(200, 25));
    // life

    if (gameState == gameStatePlaying)
    {
      // life and mana
      rectangle.setFillColor(sf::Color(190, 20, 20));
      rectangle.setPosition(sf::Vector2f(90, 622));
      rectangle.setSize(sf::Vector2f(200.0f * (float)(player->getHpDisplay()) / (float)(player->getHpMax()) , 25));
      app->draw(rectangle);

      rectangle.setFillColor(sf::Color(255, 190, 190));
      rectangle.setPosition(sf::Vector2f(90, 625));
      rectangle.setSize(sf::Vector2f(200.0f * (float)(player->getHpDisplay()) / (float)(player->getHpMax()) , 2));
      app->draw(rectangle);

      rectangle.setFillColor(sf::Color(20, 20, 190));
      rectangle.setPosition(sf::Vector2f(90, 658));
      rectangle.setSize(sf::Vector2f(200.0f * player->getPercentFireDelay() , 25));
      app->draw(rectangle);

      rectangle.setFillColor(sf::Color(190, 190, 255));
      rectangle.setPosition(sf::Vector2f(90, 661));
      rectangle.setSize(sf::Vector2f(200.0f * player->getPercentFireDelay() , 2));
      app->draw(rectangle);

      // drawing the key on the interface
      if (player->isEquiped(EQUIP_BOSS_KEY)) app->draw(keySprite);

      if (isPausing)
      {
        rectangle.setFillColor(sf::Color(0, 0, 0, 160));
        rectangle.setPosition(sf::Vector2f(OFFSET_X, OFFSET_Y));
        rectangle.setSize(sf::Vector2f(MAP_WIDTH * TILE_WIDTH, MAP_HEIGHT * TILE_HEIGHT));
        app->draw(rectangle);

        float x0 = OFFSET_X + (MAP_WIDTH / 2) * TILE_WIDTH + TILE_WIDTH / 2;
        int fade = 50 + 205 * (1.0f + cos(3.0f * getAbsolutTime())) * 0.5f;
        myText.setColor(sf::Color(255, 255, 255, fade));
        myText.setCharacterSize(40);
        myText.setString("PAUSE");
        myText.setPosition(x0 - myText.getLocalBounds().width / 2, 300);
        app->draw(myText);
      }

      if (player->isDead())
      {
        float x0 = OFFSET_X + (MAP_WIDTH / 2) * TILE_WIDTH + TILE_WIDTH / 2;
        int fade = 255 * (1.0f + cos(2.0f * getAbsolutTime())) * 0.5f;

        myText.setColor(sf::Color(255, 255, 255, 255));
        myText.setCharacterSize(25);
        myText.setString("GAME OVER");
        myText.setPosition(x0 - myText.getLocalBounds().width / 2, 400);
        app->draw(myText);

        myText.setColor(sf::Color(255, 255, 255, fade));
        myText.setCharacterSize(20);
        myText.setString("Press [ENTER] to play again !");
        myText.setPosition(x0 - myText.getLocalBounds().width / 2, 440);
        app->draw(myText);
      }
      else if (currentMap->getRoomType() == roomTypeExit)
      {
        float x0 = OFFSET_X + (MAP_WIDTH / 2) * TILE_WIDTH + TILE_WIDTH / 2;
        myText.setColor(sf::Color(255, 255, 255, 255));
        myText.setCharacterSize(25);
        myText.setString("CONGRATULATIONS !\nYou've challenged this demo and\nmanaged to kill the boss !\nSee you soon for new adventures !");
        myText.setPosition(x0 - myText.getLocalBounds().width / 2, 220);
        app->draw(myText);
      }

      if (specialState == SpecialStateFadeIn)
      {
        // fade in
        rectangle.setFillColor(sf::Color(0, 0, 0, 255 - ((FADE_IN_DELAY - timer) / FADE_IN_DELAY) * 255));
        rectangle.setPosition(sf::Vector2f(OFFSET_X, OFFSET_Y));
        rectangle.setSize(sf::Vector2f(MAP_WIDTH * TILE_WIDTH , MAP_HEIGHT * TILE_HEIGHT));
        app->draw(rectangle);
      }
      else if (specialState == SpecialStateFadeOut)
      {
        // fade out
        rectangle.setFillColor(sf::Color(0, 0, 0, ((FADE_IN_DELAY - timer) / FADE_IN_DELAY) * 255));
        rectangle.setPosition(sf::Vector2f(OFFSET_X, OFFSET_Y));
        rectangle.setSize(sf::Vector2f(MAP_WIDTH * TILE_WIDTH , MAP_HEIGHT * TILE_HEIGHT));
        app->draw(rectangle);
      }
    }

    app->display();
}

void WitchBlastGame::generateBlood(float x, float y, BaseCreatureEntity::enumBloodColor bloodColor)
{
  int nbIt;
  if (player->isEquiped(EQUIP_BLOOD_SNAKE))
    nbIt = 2;
  else
    nbIt = 1;

  for (int i=0; i < nbIt; i++)
  {
    SpriteEntity* blood = new SpriteEntity(ImageManager::getImageManager()->getImage(IMAGE_BLOOD), x, y, 16, 16, 6);
    blood->setZ(OFFSET_Y - 1);
    int b0 = 0;
    if (bloodColor == BaseCreatureEntity::bloodGreen) b0 += 6;
    blood->setFrame(b0 + rand()%6);
    blood->setType(ENTITY_BLOOD);
    blood->setVelocity(Vector2D(rand()%250));
    blood->setViscosity(0.95f);

    float bloodScale = 1.0f + (rand() % 10) * 0.1f;
    blood->setScale(bloodScale, bloodScale);
  }
}

void WitchBlastGame::showArtefactDescription(enumItemType itemType)
{
  new ArtefactDescriptionEntity(itemType, this); //, &font);
}

void WitchBlastGame::generateMap()
{
  if (currentMap->getRoomType() == roomTypeStandard)
    generateStandardMap();
  else if (currentMap->getRoomType() == roomTypeBonus)
  {
    currentMap->setCleared(true);
    Vector2D v = currentMap->generateBonusRoom();
    int bonusType = getRandomEquipItem(false);
    if (bonusType == EQUIP_FAIRY)
    {
      new ChestEntity(v.x, v.y, CHEST_FAIRY, false, this);
    }
    else
    {
      new ItemEntity( (enumItemType)(itemMagicianHat + bonusType), v.x ,v.y, this);
    }
  }
  else if (currentMap->getRoomType() == roomTypeKey)
  {
    Vector2D v = currentMap->generateKeyRoom();
    new ItemEntity( (enumItemType)(itemBossKey), v.x ,v.y, this);
    initMonsterArray();
    int x0 = MAP_WIDTH / 2;
    int y0 = MAP_HEIGHT / 2;
    monsterArray[x0][y0] = true;
    findPlaceMonsters(MONSTER_RAT, 5);
    findPlaceMonsters(MONSTER_BAT, 5);
  }
  else if (currentMap->getRoomType() == roomTypeMerchant)
  {
    currentMap->generateMerchantRoom();

    ItemEntity* item1 = new ItemEntity(
      itemHealth,
      OFFSET_X + (MAP_WIDTH / 2 - 2) * TILE_WIDTH + TILE_WIDTH / 2,
      OFFSET_Y + (MAP_HEIGHT / 2) * TILE_HEIGHT,
      this);
    item1->setMerchandise(true);

    int bonusType = getRandomEquipItem(true);
    ItemEntity* item2 = new ItemEntity(
      (enumItemType)(itemMagicianHat + bonusType),
      OFFSET_X + (MAP_WIDTH / 2 + 2) * TILE_WIDTH + TILE_WIDTH / 2,
      OFFSET_Y + (MAP_HEIGHT / 2) * TILE_HEIGHT,
      this);
    item2->setMerchandise(true);

    new PnjEntity(OFFSET_X + (MAP_WIDTH / 2) * TILE_WIDTH + TILE_WIDTH / 2,
                  OFFSET_Y + (MAP_HEIGHT / 2 - 2) * TILE_HEIGHT,
                  0);

    currentMap->setCleared(true);
  }
  else if (currentMap->getRoomType() == roomTypeBoss)
  {
    currentMap->generateRoom(0);

    boss = new KingRatEntity(OFFSET_X + (MAP_WIDTH / 2 - 2) * TILE_WIDTH + TILE_WIDTH / 2,
                      OFFSET_Y + (MAP_HEIGHT / 2 - 2) * TILE_HEIGHT + TILE_HEIGHT / 2,
                      this);
  }
  else if (currentMap->getRoomType() == roomTypeStarting)
  {
    currentMap->generateRoom(0);
    currentMap->setCleared(true);
    int bonusType = getRandomEquipItem(false);
    if (bonusType == EQUIP_FAIRY)
    {
      new ChestEntity(OFFSET_X + (TILE_WIDTH * MAP_WIDTH * 0.5f),
                                           OFFSET_Y + 120.0f + (TILE_HEIGHT * MAP_HEIGHT * 0.5f),
                                           CHEST_FAIRY, false, this);
    }
    else
    {
      new ItemEntity( (enumItemType)(itemMagicianHat + bonusType),
                          OFFSET_X + (TILE_WIDTH * MAP_WIDTH * 0.5f),
                          OFFSET_Y + 120.0f + (TILE_HEIGHT * MAP_HEIGHT * 0.5f),
                          this);
    }
  }
  else if (currentMap->getRoomType() == roomTypeExit)
  {
    currentMap->generateRoom(0);
    currentMap->setCleared(true);
  }
  else
    currentMap->randomize(currentMap->getRoomType());
}

void WitchBlastGame::initMonsterArray()
{
  for (int i = 0; i < MAP_WIDTH; i++)
    for (int j = 0; j < MAP_HEIGHT; j++)
      monsterArray[i][j] = false;
}

void WitchBlastGame::addMonster(monster_type_enum monsterType, float xm, float ym)
{
  switch (monsterType)
  {
    case MONSTER_RAT: new RatEntity(xm, ym - 2, this); break;
    case MONSTER_BAT: new BatEntity(xm, ym, this); break;
    case MONSTER_EVIL_FLOWER: new EvilFlowerEntity(xm, ym, this); break;
    case MONSTER_SLIME: new SlimeEntity(xm, ym, this); break;

    case MONSTER_KING_RAT: new KingRatEntity(xm, ym, this); break;
  }
}

void WitchBlastGame::findPlaceMonsters(monster_type_enum monsterType, int amount)
{
  // find a suitable place
  bool isMonsterFlying = monsterType == MONSTER_BAT;

  bool bOk;
  int xm, ym;
  float xMonster, yMonster;

  for (int index = 0; index < amount; index++)
  {
    bOk = false;

    while (!bOk)
    {
      bOk = true;
      xm = 1 +rand() % (MAP_WIDTH - 3);
      ym = 1 +rand() % (MAP_HEIGHT - 3);
      if (monsterArray[xm][ym])
      {
        bOk = false;
      }
      if (bOk && !isMonsterFlying && !currentMap->isWalkable(xm, ym))
      {
        bOk = false;
      }
      if (bOk)
      {
        xMonster = OFFSET_X + xm * TILE_WIDTH + TILE_WIDTH * 0.5f;
        yMonster = OFFSET_Y + ym * TILE_HEIGHT+ TILE_HEIGHT * 0.5f;

        float dist2 = (xMonster - player->getX())*(xMonster - player->getX()) + (yMonster - player->getY())*(yMonster - player->getY());
        if ( dist2 < 75000.0f)
        {
          bOk = false;
        }
        else
        {
          addMonster(monsterType, xMonster, yMonster);
          monsterArray[xm][ym] = true;
        }
      }
    }
  }
}

void WitchBlastGame::generateStandardMap()
{
  initMonsterArray();

  int random = rand() % 100;

  if (random < 16)
  {
    currentMap->generateRoom(rand()%3);
    findPlaceMonsters(MONSTER_RAT,4);
  }
  else if (random < 32)
  {
    currentMap->generateRoom(rand()%4);
    findPlaceMonsters(MONSTER_BAT,4);
  }
  else if (random < 48)
  {
    currentMap->generateRoom(rand()%4);
    findPlaceMonsters(MONSTER_EVIL_FLOWER,4);
  }
  else if (random < 64)
  {
    Vector2D v = currentMap->generateBonusRoom();
    new ChestEntity(v.x, v.y, CHEST_BASIC, false, this);
    currentMap->setCleared(true);
  }
  else if (random < 80)
  {
    currentMap->generateRoom(rand()%3);
    findPlaceMonsters(MONSTER_RAT,3);
    findPlaceMonsters(MONSTER_BAT,3);
  }
  else
  {
    currentMap->generateRoom(rand()%3);
    findPlaceMonsters(MONSTER_SLIME,8 + rand() % 5);
  }
}

int WitchBlastGame::getRandomEquipItem(bool toSale = false)
{
  std::vector<int> bonusSet;
  int setSize = 0;
  for (int i = 0; i < NUMBER_EQUIP_ITEMS; i++)
  {
    if (!player->isEquiped(i) && i != EQUIP_BOSS_KEY)
    {
      if (!toSale || i!= EQUIP_FAIRY)
      {
        bonusSet.push_back(i);
        setSize++;
      }
    }
  }
  int bonusType = 0;
  if (setSize > 0) bonusType = bonusSet[rand() % setSize];

  return bonusType;
}

void WitchBlastGame::verifyDoorUnlocking()
{
  int colliding = (player->getColliding());

  if (colliding > 0 && currentMap->isCleared() && !bossRoomOpened && player->isEquiped(EQUIP_BOSS_KEY))
  {
    int xt = (player->getX() - OFFSET_X) / TILE_WIDTH;
    int yt = (player->getY() - OFFSET_Y) / TILE_HEIGHT;

    if (yt <= 1 && xt >= MAP_WIDTH / 2 - 1 && xt <= MAP_WIDTH / 2 + 1 && currentMap->hasNeighbourUp() == 2)
    {
      doorEntity[0]->openDoor();
      currentMap->setTile(MAP_WIDTH / 2, 0, 0);
      SoundManager::getSoundManager()->playSound(SOUND_DOOR_OPENING);
      player->useBossKey();
      bossRoomOpened = true;
    }
    if (yt >= MAP_HEIGHT - 2 && xt >= MAP_WIDTH / 2 - 1 &&xt <= MAP_WIDTH / 2 + 1 && currentMap->hasNeighbourDown() == 2)
    {
      doorEntity[2]->openDoor();
      currentMap->setTile(MAP_WIDTH / 2, MAP_HEIGHT - 1, 0);
      SoundManager::getSoundManager()->playSound(SOUND_DOOR_OPENING);
      player->useBossKey();
      bossRoomOpened = true;
    }
    if (xt <= 1 && yt >= MAP_HEIGHT / 2 - 1 && yt <= MAP_HEIGHT / 2 + 1 && currentMap->hasNeighbourLeft() == 2)
    {

      doorEntity[1]->openDoor();
      currentMap->setTile(0, MAP_HEIGHT / 2, 0);
      SoundManager::getSoundManager()->playSound(SOUND_DOOR_OPENING);
      player->useBossKey();
      bossRoomOpened = true;
    }
    if (xt >= MAP_WIDTH - 2 && yt >= MAP_HEIGHT / 2 - 1 && yt <= MAP_HEIGHT / 2 + 1 && currentMap->hasNeighbourRight() == 2)
    {
      doorEntity[3]->openDoor();
      currentMap->setTile(MAP_WIDTH - 1, MAP_HEIGHT / 2, 0);
      SoundManager::getSoundManager()->playSound(SOUND_DOOR_OPENING);
      player->useBossKey();
      bossRoomOpened = true;
    }
  }
}

void WitchBlastGame::playMusic(musicEnum musicChoice)
{
  music.stop();
  music.setLoop(true);
  bool ok = false;
  switch (musicChoice)
  {
  case MusicDonjon:
    ok = music.openFromFile("media/sound/track00.ogg");
    music.setVolume(75);
    break;

  case MusicEnding:
    ok = music.openFromFile("media/sound/track_ending.ogg");
    music.setVolume(35);
    break;
  }

  if (ok)
    music.play();
}

void WitchBlastGame::saveGame()
{
  ofstream file("game.sav", ios::out | ios::trunc);

  int i, j, k, l;

  if (file)
  {
    // floor
    int nbRooms = 0;
    for (j = 0; j < FLOOR_HEIGHT; j++)
    {
      for (i = 0; i < FLOOR_WIDTH; i++)
      {
        file << currentFloor->getRoom(i,j) << " ";
        if (currentFloor->getRoom(i,j) > 0) nbRooms++;
      }
      file << std::endl;
    }

    // maps
    saveMapItems();

    file << nbRooms << std::endl;
    for (j = 0; j < FLOOR_HEIGHT; j++)
    {
      for (i = 0; i < FLOOR_WIDTH; i++)
      {
        if (currentFloor->getRoom(i,j) > 0)
        {
          file << i << " " << j << " "
          << currentFloor->getMap(i, j)->getRoomType() << " "
          << currentFloor->getMap(i, j)->isKnown() << " "
          << currentFloor->getMap(i, j)->isVisited() << " "
          << currentFloor->getMap(i, j)->isCleared() << std::endl;
          if (currentFloor->getMap(i, j)->isVisited())
          {
            for (l = 0; l < MAP_HEIGHT; l++)
            {
              for (k = 0; k < MAP_WIDTH; k++)
              {
                file << currentFloor->getMap(i, j)->getTile(k, l) << " ";
              }
              file << std::endl;
            }
            // items, etc...
            std::list<DungeonMap::itemListElement> itemList = currentFloor->getMap(i, j)->getItemList();
            file << itemList.size() << std::endl;
            std::list<DungeonMap::itemListElement>::iterator it;
            for (it = itemList.begin (); it != itemList.end ();)
            {
              DungeonMap::itemListElement ilm = *it;
              it++;

              file << ilm.type << " " << ilm.x << " " << ilm.y << " " << ilm.merch << std::endl;
            }

            // chests
            std::list<DungeonMap::chestListElement> chestList = currentFloor->getMap(i, j)->getChestList();
            file << chestList.size() << std::endl;
            std::list<DungeonMap::chestListElement>::iterator itc;
            for (itc = chestList.begin (); itc != chestList.end ();)
            {
              DungeonMap::chestListElement ilm = *itc;
              itc++;

              file << ilm.type << " " << ilm.x << " " << ilm.y << " " << ilm.state << std::endl;
            }

            // sprites
            std::list<DungeonMap::spriteListElement> spriteList = currentFloor->getMap(i, j)->getSpriteList();
            file << spriteList.size() << std::endl;
            std::list<DungeonMap::spriteListElement>::iterator its;
            for (its = spriteList.begin (); its != spriteList.end ();)
            {
              DungeonMap::spriteListElement ilm = *its;
              its++;

              file << ilm.type << " " << ilm.frame << " " << ilm.x << " " << ilm.y << " " << ilm.scale << std::endl;
            }
          }
        }
      }
      file << std::endl;
    }

    // game
    file << floorX << " " << floorY << std::endl;
    file << bossRoomOpened << std::endl;
    // boss door !

    // player
    file << player->getHp() << " " << player->getHpMax() << " " << player->getGold() << std::endl;
    for (i = 0; i < NUMBER_EQUIP_ITEMS; i++) file << player->isEquiped(i) << " ";
    file << std::endl;
    file << player->getX() << " " << player->getY() << std::endl;

    file.close();
  }
  else
  {
    cerr << "[ERROR] Saving the game..." << endl;
  }
}

bool WitchBlastGame::loadGame()
{
  ifstream file("game.sav", ios::in);

  if (file)
  {
    int i, j, k, n;

    // floor
    currentFloor = new GameFloor();
    for (j = 0; j < FLOOR_HEIGHT; j++)
    {
      for (i = 0; i < FLOOR_WIDTH; i++)
      {
        int n;
        file >> n;
        currentFloor->setRoom(i, j, n);
      }
    }

    // maps
    int nbRooms;
    file >> nbRooms;

    for (k = 0; k < nbRooms; k++)
    {
      file >> i;
      file >> j;
      file >> n;
      DungeonMap* iMap = new DungeonMap(currentFloor, i, j);
      currentFloor->setMap(i, j, iMap);
      iMap->setRoomType((roomTypeEnum)n);
      bool flag;
      file >> flag;
      iMap->setKnown(flag);
      file >> flag;
      iMap->setVisited(flag);
      file >> flag;
      iMap->setCleared(flag);

      if (iMap->isVisited())
      {
        for (j = 0; j < MAP_HEIGHT; j++)
        {
          for (i = 0; i < MAP_WIDTH; i++)
          {
            file >> n;
            iMap->setTile(i, j, n);
          }
        }
        // items int the map
        file >> n;
        for (i = 0; i < n; i++)
        {
          int t;
          float x, y;
          bool merc;
          file >> t >> x >> y >> merc;
          iMap->addItem(t, x, y, merc);
        }
        // chests in the map
        file >> n;
        for (i = 0; i < n; i++)
        {
          int t;
          float x, y;
          bool state;
          file >> t >> x >> y >> state;
          iMap->addChest(t, state, x, y);
        }
        // sprites in the map
        file >> n;
        for (i = 0; i < n; i++)
        {
          int t, f;
          float x, y, scale;
          file >> t >> f >> x >> y >> scale;
          iMap->addSprite(t, f, x, y, scale);
        }
      }
    }

    //currentFloor->displayToConsole();

    // game
    file >> floorX >> floorY;
    currentMap = currentFloor->getMap(floorX, floorY);
    file >> bossRoomOpened;

    // player
    int hp, hpMax, gold;
    file >> hp >> hpMax >> gold;
    player = new PlayerEntity(ImageManager::getImageManager()->getImage(0),
                              this,
                              OFFSET_X + (TILE_WIDTH * MAP_WIDTH * 0.5f),
                              OFFSET_Y + (TILE_HEIGHT * MAP_HEIGHT * 0.5f));
    player->setHp(hp);
    player->setHpMax(hpMax);
    player->setGold(gold);
    for (i = 0; i < NUMBER_EQUIP_ITEMS; i++)
    {
      bool eq;
      file >> eq;
      player->setEquiped(i, eq);
    }
    float x, y;
    file >> x >> y;

    player->moveTo(x, y);

    file.close();
    remove("game.sav");
  }
  else
  {
    return false;
  }

  return true;
}
