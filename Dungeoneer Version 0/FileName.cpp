#include <iostream>  // Used for cin and cout
#include <fstream>  // Used for loading in the rooms from a file
#include <nlohmann/json.hpp>  // Used for handling the room data
#include <vector>  // Used for dynamic arrays ig
#include <random>  // Used for room generation and attacks
// #include <windows.h>  // Idk where this is used so I'm commenting it out rn
#include <memory> // Used for (ughh) shared pointers //Update, I hate shared pointers with a passion now and am going to make it my goal to delete them from existence

// Here temporarily, possibly remove later
using namespace std;
using json = nlohmann::json;


// Loads the json data in
ifstream File("Rooms.json");
json allRooms = json::parse(File);

// Stuff for rng
random_device randDevice;
mt19937 generatorEngine(randDevice());

int randint(int min, int max) {			// Returns a random number in the range using mt19937
	uniform_int_distribution<> RandomNumber(min, max);
	return RandomNumber(generatorEngine);

}




uniform_int_distribution<> RandRoom(1, allRooms.size() - 1);

//Class & Struct Prototypes
class Entity;
class Player;
struct Direction;
class RoomNode;
class Dungeon;


//Classes & Structs Definitions
class Entity {
public:
	string Name;
	int ID;
};


class Player : public Entity {
public:
	Player(){}
	Player(string name) {
		Name = name;
		ID = 0;
	}
};


struct Direction {
	string dirName;
	weak_ptr<RoomNode> roomInDir;

	Direction(string Name) {
		dirName = Name;
		//roomInDir = nullptr;
	}
};


class RoomNode {
	json RoomData;
	int timeAlive = 0;		// Each unit of time is theoretically a tenth of a second, however this is not strictly enforced

public:
	vector<weak_ptr<Entity>> RoomInhabitants;
	vector<Direction> ConnectedRooms;

	RoomNode(json jData) { // Should only be called when starting a new dungeon

		//cout << "\n\nROOM MADE FROM JSON\n\n";

		if (jData.contains("Description") && jData.contains("Looks") && jData.contains("Exits") && jData.contains("Attributes")) {		// If the room has the appropriate fields

			if (jData["Exits"].size() >= 1) {
				RoomData = jData;

				while (ConnectedRooms.size() < RoomData["Exits"].size()) {
					ConnectedRooms.push_back(Direction(RoomData["Exits"][ConnectedRooms.size()]));			// Adds a new direction for each possible exit from the room
				}

			}
			else {
				cout << "\n\n-------------ROOM GENERATION FAILED-------------\nRoom has no exits\n";

				if (jData.contains("Name")) {		// If the room has an name (which all rooms should), it displays the room name to help with debugging
					cout << "Room name : " << jData["Name"] << "\n";
				}
				else {
					cout << "Nameless Room\n";
				}

				delete this;
			}

		}
		else {
			cout << "\n\n-------------ROOM GENERATION FAILED-------------\nData doesn't contain required fields\n";

			if (jData.contains("Name")) {		// If the room has an ID (which all rooms should), it displays the room ID to help with debugging
				cout << "Room name : " << jData["Name"] << "\n";
			}
			else {
				cout << "Nameless Room\n";
			}
			delete this;
		}

	}


	//~RoomNode();


	RoomNode(shared_ptr<RoomNode> creatorRoom) { // Should be called when making subsequent rooms

		json jData = allRooms[to_string(RandRoom(generatorEngine))];
		//json jData = allRooms["4"];

		//cout << "\n\nRoom Being Made: \n";
		//cout << "Creator Name: " << creatorRoom->GetRoomData()["Name"] << "\n" << jData;

		if (jData.contains("Description") && jData.contains("Looks") && jData.contains("Exits") && jData.contains("Attributes")) {		// If the room has the appropriate fields


			if (jData["Exits"].size() > 1) {	// If there are multiple exits, randomly links one to the creator room

				RoomData = jData;

				int returningDir = randint(0, RoomData["Exits"].size() - 1);

				while (ConnectedRooms.size() < RoomData["Exits"].size()) {
					ConnectedRooms.push_back(Direction(RoomData["Exits"][ConnectedRooms.size()]));			// Adds a new direction for each possible exit from the room
				}

				ConnectedRooms[returningDir].roomInDir = creatorRoom;

			}
			else if (jData["Exits"].size() == 1) {	// If there is only one exit, sets that one exit to the room that the entity was originally from
				
				RoomData = jData;
				ConnectedRooms.push_back(Direction(string(RoomData["Exits"][0])));
				ConnectedRooms[0].roomInDir = creatorRoom;
			}
			else{
				cout << "\n\n-------------ROOM GENERATION FAILED-------------\nRoom has no exits\n";

				if (jData.contains("Name")) {		// If the room has an ID (which all rooms should), it displays the room ID to help with debugging
					cout << "Room name : " << jData["Name"] << "\n";
				}
				else {
					cout << "Nameless Room\n";
				}

				delete this;
			}

		}
		else {
			cout << "\n\n-------------ROOM GENERATION FAILED-------------\nData doesn't contain required fields\n";

			if (jData.contains("Name")) {		// If the room has an ID (which all rooms should), it displays the room ID to help with debugging
				cout << "Room name : " << jData["Name"] << "\n";
			}
			else {
				cout << "Nameless Room\n";
			}

			delete this;
		}

	}


	void UpdateAge(int time) {
		timeAlive += time;
	}

	int GetTimeAlive() {
		return timeAlive;
	}

	json GetRoomData() { // Returns the json variable that contains all of the room's information
		return RoomData;
	}

	int InhabitantsNumber() {
		//cout << RoomInhabitants.size();
		return RoomInhabitants.size();
	}
};


class Dungeon {
	vector<shared_ptr<RoomNode>> Rooms;
	vector<shared_ptr<Entity>> EntitiesLoaded;

	int timeAlive = 0;		// Each unit of time is theoretically a tenth of a second, however this is not strictly enforced

public:


	shared_ptr<RoomNode> CurrentRoom;

	shared_ptr<Player> PlayerCharacter;

	void UpdateDungeon(int time) {		// Checks all rooms loaded and updates them as appropriate

		bool ConnectedEntity;
		bool RoomsSuccessfullyLoaded;
		shared_ptr<RoomNode> TempPointer;

		if (time) { 
			timeAlive += time;
		}

		for (int o = 0; o < Rooms.size(); o++) {		// Goes through every room

			ConnectedEntity = true;		// Used to determine whether a room should exist or not
			Rooms[o]->UpdateAge(time);

			//cout << "\n" << Rooms[o]->GetRoomData()["Name"] << "\n";

			if (Rooms[o]->RoomInhabitants.size() == 0) {
				ConnectedEntity = false;
				//cout << "\nConnected Room Size" << Rooms[o]->ConnectedRooms.size() << "\n";
				for (int i = 0; i < Rooms[o]->ConnectedRooms.size(); i++) {		// Goes through every room connected to the current room
					//cout << "\ni: " << i << "\n";
					if (!Rooms[o]->ConnectedRooms[i].roomInDir.expired()) {
						//cout << "\n\n" << Rooms[o]->ConnectedRooms[i].roomInDir->GetRoomData()["Name"] << " is " << Rooms[o]->ConnectedRooms[i].dirName << " of " << Rooms[o]->GetRoomData()["Name"];
						//cout << Rooms[o]->ConnectedRooms[i].roomInDir->InhabitantsNumber();
						if (Rooms[o]->ConnectedRooms[i].roomInDir.lock()->RoomInhabitants.size() != 0) {
							ConnectedEntity = true;
							
							////cout << "\nROOM WITH INHABITANT FOR " << Rooms[o]->GetRoomData()["Name"] << ": " << Rooms[o]->ConnectedRooms[i].roomInDir->GetRoomData()["Name"] << "  ---------------- INHABITANT ID: " << Rooms[o]->ConnectedRooms[i].roomInDir->RoomInhabitants[0]->ID << "\n";

						}
					}
				}

				if (ConnectedEntity == false) {
					////cout << "\n\nDELETING " << Rooms[o]->GetRoomData()["Name"] << "\n\n";
					Rooms.erase(Rooms.begin() + o);		// Deletes the room if there are no inhabitants and no entities nearby
				}
			}
			else {		// Runs if the room has an inhabitant


				RoomsSuccessfullyLoaded = false;	// Used to check for every room that has an inhabitant, yet the connected rooms haven't been constructed yet

				while (!RoomsSuccessfullyLoaded) {
					RoomsSuccessfullyLoaded = true;

					for (int i = 0; i < Rooms[o]->ConnectedRooms.size(); i++) {		// Goes through every direction of the connected room
						if (Rooms[o]->ConnectedRooms[i].roomInDir.expired()) {		// If the direction is null, despite the room having an inhabitant, it makes a room in that direction
							
							//cout << "\n\nMAKING ROOM\n\n";
							
							
							RoomsSuccessfullyLoaded = false;

							Rooms.push_back(make_shared<RoomNode>(Rooms[o]));

							Rooms[o]->ConnectedRooms[i].roomInDir = (Rooms[Rooms.size() - 1]);
							//cout << "\nRoom count:" << Rooms.size() << "\n";
						}
					}
				}

			}
		}
	}

	Dungeon() {

		string PlayerName;

		cout << "Please enter a name for your character: ";
		cin >> PlayerName;

		PlayerCharacter = make_shared<Player>(PlayerName);

		cout << "\n";
	}

	void ChangeRoom(string direction) {		// Moves the player

		////cout << "\nDirEnter: " << direction << "\n";
		
		int layers = 0;

		////cout << "\nCHECKING ROOMS";
		for (int o = 0; o < Rooms.size(); o++) {		// Goes through every room with counter o
			
			cout << "\n" << o << ". " << Rooms[o]->GetRoomData()["Name"];

			for (int i = 0; i < Rooms[o]->RoomInhabitants.size(); i++) {		// Goes through every entity in the room with counter i

				cout << "\n" << Rooms[o]->GetRoomData()["Name"] << ": " << Rooms[o]->RoomInhabitants[i].lock()->Name;

				if (!Rooms[o]->RoomInhabitants[i].expired()) {			// If the inhabitants vector is associated

					if (Rooms[o]->RoomInhabitants[i].lock()->ID == 0) {		// If the entity moving is found

						if (layers < 1) { layers = 1; }

						for (int j = 0; j < Rooms[o]->ConnectedRooms.size(); j++) {		// Goes through every direction/exit with counter j

							////cout << "\nDirName: " << Rooms[o]->ConnectedRooms[j].dirName << "\n";
							//cout << "\nRoomInDir\n" << Rooms[o]->ConnectedRooms[j].roomInDir->GetRoomData() << "\n";

							if (Rooms[o]->ConnectedRooms[j].dirName == direction) {		// If the exit matches the user inputted exit

								if (layers < 2) { layers = 2; }

								//cout << Rooms[o]->ConnectedRooms[j].dirName << "\n";

								CurrentRoom = Rooms[o]->ConnectedRooms[j].roomInDir.lock();

								Rooms[o]->ConnectedRooms[j].roomInDir.lock()->RoomInhabitants.push_back(Rooms[o]->RoomInhabitants[i]);		// Adds the moving entity to the list of inhabitants at the destination room

								Rooms[o]->RoomInhabitants.erase(Rooms[o]->RoomInhabitants.begin() + i);		// Removes the moving entity from the list of inhabitants at the original room


								//cout << "\nCurRoom\n" << CurrentRoom->GetRoomData() << "\n";;

								UpdateDungeon(50);

								return;		// Returns the current room

							}
						}
					}
				}
			}

		}

		if (layers == 1) { cout << "EXIT NOT FOUND WHEN MOVING\n"; }
		if (layers == 0) { cout << "ROOM OR INHABITANT NOT FOUND WHEN MOVING\n"; }

		return;		// Returns nullptr so the room doesn't change

	}

	shared_ptr<Player> Initialise() {

		Rooms.push_back(make_shared<RoomNode>(RoomNode(allRooms["0"])));
		Rooms[0]->RoomInhabitants.push_back(make_shared<Entity>(PlayerCharacter));

		CurrentRoom = Rooms[0];

		//cout << "\n\n" << CurrentRoom << " " << &Rooms[0];

		UpdateDungeon(0);

		cout << "Dungeoneer Initialised \n";

		return make_shared<Player>(PlayerCharacter);
	}

	int RoomsNumber() {
		return Rooms.size();
	}
	
	int EntitiesNumber() {
		return EntitiesLoaded.size();
	}

	string RoomName(int index) {
		return Rooms[index]->GetRoomData()["Name"];
	}

	void test() {

		cout << "\n\n" << CurrentRoom << " " << &Rooms[0];
		cout << "\n\n" << CurrentRoom->GetTimeAlive() << " " << Rooms[0]->GetTimeAlive();
		//if (CurrentRoom->ConnectedRooms == Rooms[0].ConnectedRooms) {
		//	return true;
		//}
		//else {
		//	return false;
		//}
	}

	void updTimeAli(int index, int time) {
		Rooms[index]->UpdateAge(time);
	}
	

};


// Function prototypes
void lowercase(string &text);
vector<string> split(string Input, char delimiter);
bool isInteger(string input);
string capitalise(string input);
void processInput(string command);
void generateRooms();
void look(vector<string> command);
void move(vector<string> command);
void other(vector<string> command);
void displayDebugInfo();

//Not super important things that I'm just going to put here in global variables, even though they probably shouldn't be here.
int invalidInputs = 0;
bool timesUsedHelp = 0;

// Super important things that also probably shouldn't be global.
Dungeon TheDungeon;
vector<Entity> EntitiesLoaded;


int main() {
	shared_ptr<Player> player;

	string userInput = " ";

	player = make_shared<Player>(TheDungeon.PlayerCharacter);

	TheDungeon.Initialise();

	cout << "\n" << "With a pounding headache, you wake up in a dimly lit room. Your back aches from lying on the stone floor, slightly damp from the puddles on the floor. Around you are torches on each of the four stone walls, and there are two arched openings to either side of you. Looking at your compass, one is east and one is west. \n";

	while (true) {
		if (userInput != "") {
			cout << "\n" << player->Name << ":\\Dungeon>";
		}
			getline(cin, userInput);
			//lowercase(userInput);
			//SplitInput = split(userInput, ' ');

			//cout << SplitInput.size();

			//for (int i = 0; i < SplitInput.size(); i++) {
				//cout << "Split " << i<< " = " << SplitInput[i];
			//}

			processInput(userInput);

	}

}

void lowercase(string &text) {

	for (int i = 0; i < text.size(); i++) {
		text[i] = tolower(text[i]);
	}

	//cout << text;	

}


vector<string> split(string Input, char delimiter) {

	//cout << "Counter" << counter;

	vector<string> splitString;
	int startPoint = 0;
	int length = 0;

	//cout << "\n" << Input << "\n";

	for (int i = 0; i < Input.size(); i++) {

		//cout << i << "{" << Input[i] << " - " << delimiter << "\n";

		if (Input[i] == delimiter) {
			//cout << "--- " << Input.substr(startPoint, length);
			splitString.push_back(Input.substr(startPoint, length));
			startPoint = i+1;
			length = 0;
		}
		else {
			length += 1;
		}
	}

	splitString.push_back(Input.substr(startPoint, length + 1));

	return splitString;
}


bool isInteger(string input) {
	if (input != "") {
		for (int i = 0; i < input.size(); i++) {
			if (!isdigit(input[i])) {
				return false;
			}
		}
		return true;
	}
	else {
		return false;
	}
}


string capitalise(string input) {
	char FirstLetter = toupper(input[0]);
	return FirstLetter + input.substr(1);
}


void processInput(string command) {


	vector<string> SplitCommand;
	lowercase(command);
	SplitCommand = split(command, ' ');

	if (SplitCommand.size() == 0) {
		return;
	}

	if (SplitCommand[0] == "help") {
		cout << "You still feel groggy. Your memories are foggy, but you could try |RECALL| what's happening and why you're here. You could also |LOOK| around at <something>. Additionally, you could |MOVE| to another room through an <exit>. \n";
	}
	else if (SplitCommand[0] == "look") {
		look(SplitCommand);
	}
	else if (SplitCommand[0] == "move") {
		move(SplitCommand);
	}
	else {
		other(SplitCommand);
	}

}




void generateRooms() {

}


void look(vector<string> command) {

	bool found = false;

	switch (command.size()) {
	case 1:
		cout << string(TheDungeon.CurrentRoom->GetRoomData()["Looks"]["#AROUND"]);
		break;
	case 2:
		
		for (int i = 0; i < TheDungeon.CurrentRoom->ConnectedRooms.size(); i++) {
			if (command[1] == TheDungeon.CurrentRoom->ConnectedRooms[i].dirName) {
				found = true;
				if (TheDungeon.CurrentRoom->ConnectedRooms[i].roomInDir.lock()->GetRoomData()["Looks"].contains("#INTO")) {
					cout << string(TheDungeon.CurrentRoom->ConnectedRooms[i].roomInDir.lock()->GetRoomData()["Looks"]["#INTO"]);
				}
				else {
					cout << "---------- NO #INTO FOUND AT ROOM ------------------\nRoom Name: " << TheDungeon.CurrentRoom->ConnectedRooms[i].roomInDir.lock()->GetRoomData()["Name"];
				}
			}
		}

		if (not found) {

			if (command[1] == "room" || command[1] == "around") {
				cout << string(TheDungeon.CurrentRoom->GetRoomData()["Looks"]["#AROUND"]);
			}
			else if (TheDungeon.CurrentRoom->GetRoomData()["Looks"].contains(command[1])) {
				cout << string(TheDungeon.CurrentRoom->GetRoomData()["Looks"][command[1]]);
			}
			else if (TheDungeon.CurrentRoom->GetRoomData()["Looks"].contains("#DEFAULTOVERRIDE")) {
				cout << string(TheDungeon.CurrentRoom->GetRoomData()["Looks"]["#DEFAULTOVERRIDE"]);
			}
			else {
				cout << "(You either don't see that, or I don't understand you.)\n";
			}
		}
		break;

	case 3:
		if (command[1] == "at") {

			for (int i = 0; i < TheDungeon.CurrentRoom->ConnectedRooms.size(); i++) {
				if (command[2] == TheDungeon.CurrentRoom->ConnectedRooms[i].dirName) {
					found = true;
					if (TheDungeon.CurrentRoom->ConnectedRooms[i].roomInDir.lock()->GetRoomData()["Looks"].contains("#INTO")) {
						cout << string(TheDungeon.CurrentRoom->ConnectedRooms[i].roomInDir.lock()->GetRoomData()["Looks"]["#INTO"]);
					}
					else {
						cout << "---------- NO #INTO FOUND AT ROOM ------------------\nRoom Name: " << TheDungeon.CurrentRoom->ConnectedRooms[i].roomInDir.lock()->GetRoomData()["Name"];
					}
				}
			}

			if (not found) {

				if (command[2] == "room" || command[2] == "around") {
					cout << string(TheDungeon.CurrentRoom->GetRoomData()["Looks"]["#AROUND"]);
				}
				else if (TheDungeon.CurrentRoom->GetRoomData()["Looks"].contains(command[2])) {
					cout << string(TheDungeon.CurrentRoom->GetRoomData()["Looks"][command[2]]);
				}
				else if (TheDungeon.CurrentRoom->GetRoomData()["Looks"].contains("#DEFAULTOVERRIDE")) {
					cout << string(TheDungeon.CurrentRoom->GetRoomData()["Looks"]["#DEFAULTOVERRIDE"]);
				}
				else {
					cout << "(You either don't see that, or I don't understand you.)\n";
				}
			}
			break;
		}

	case 4:

		if (command[1] == "at" && command[2] == "the") {
			for (int i = 0; i < TheDungeon.CurrentRoom->ConnectedRooms.size(); i++) {
				if (command[3] == TheDungeon.CurrentRoom->ConnectedRooms[i].dirName) {
					found = true;
					if (TheDungeon.CurrentRoom->ConnectedRooms[i].roomInDir.lock()->GetRoomData()["Looks"].contains("#INTO")) {
						cout << string(TheDungeon.CurrentRoom->ConnectedRooms[i].roomInDir.lock()->GetRoomData()["Looks"]["#INTO"]);
					}
					else {
						cout << "---------- NO #INTO FOUND AT ROOM ------------------\nRoom Name: " << TheDungeon.CurrentRoom->ConnectedRooms[i].roomInDir.lock()->GetRoomData()["Name"];
					}
				}
			}

			if (not found) {

				if (command[3] == "room" || command[3] == "around") {
					cout << string(TheDungeon.CurrentRoom->GetRoomData()["Looks"]["#AROUND"]);
				}
				else if (TheDungeon.CurrentRoom->GetRoomData()["Looks"].contains(command[3])) {
					cout << string(TheDungeon.CurrentRoom->GetRoomData()["Looks"][command[3]]);
				}
				else if (TheDungeon.CurrentRoom->GetRoomData()["Looks"].contains("#DEFAULTOVERRIDE")) {
					cout << string(TheDungeon.CurrentRoom->GetRoomData()["Looks"]["#DEFAULTOVERRIDE"]);
				}
				else {
					cout << "(You either don't see that, or I don't understand you.)\n";
				}
			}
			break;
		}
	default:
		cout << "(You either don't see that, or I don't understand you.)\n";
	}

}


void move(vector<string> command) {

	string roomSelection;
	bool found = false;

	switch (command.size()) {
	case 1:
		cout << "Where do you want to go?: \n";

		//cout << "CURRENT ROOM: " << TheDungeon.CurrentRoom->GetRoomData()["Name"] << "\n";

		for (int i = 0; i < TheDungeon.CurrentRoom->GetRoomData()["Exits"].size(); i++) {

			cout << i + 1 << ": " << capitalise(string(TheDungeon.CurrentRoom->GetRoomData()["Exits"][i])) << "\n";
		}
		cin >> roomSelection;

		if (isInteger(roomSelection)) {
			if (stoi(roomSelection) - 1 < TheDungeon.CurrentRoom->GetRoomData()["Exits"].size() && stoi(roomSelection) > 0) {		// If the user entered value isn't greater than the number of exits, and is greater than 0
				
				TheDungeon.ChangeRoom(TheDungeon.CurrentRoom->GetRoomData()["Exits"][stoi(roomSelection) - 1]);
				
				cout << string(TheDungeon.CurrentRoom->GetRoomData()["Description"]);

			}
			else {
				cout << "Invalid Exit. \n";
			}

		}
		else {
			lowercase(roomSelection);

			cout << "\n" << TheDungeon.CurrentRoom->GetRoomData()["Exits"] << "\n";

			for (int i = 0; i < TheDungeon.CurrentRoom->GetRoomData()["Exits"].size(); i++) {

				if (TheDungeon.CurrentRoom->GetRoomData()["Exits"][i] == roomSelection) {
					TheDungeon.ChangeRoom(roomSelection);

					cout << string(TheDungeon.CurrentRoom->GetRoomData()["Description"]);
					found = true;
				}

			}

			if (not found) {
				cout << "Invalid Exit. \n";
			}

		}
		break;


	case 2:
		if (isInteger(command[1])) {
			if (stoi(command[1]) - 1 < TheDungeon.CurrentRoom->GetRoomData()["Exits"].size() && stoi(command[1]) > 0) {

				TheDungeon.ChangeRoom(TheDungeon.CurrentRoom->GetRoomData()["Exits"][stoi(roomSelection) - 1]);

				cout << string(TheDungeon.CurrentRoom->GetRoomData()["Description"]);

			}
			else {
				cout << "Invalid Exit. \n";
			}

		}
		else {

			if (TheDungeon.CurrentRoom->GetRoomData()["Exits"].contains(roomSelection)) {
				TheDungeon.ChangeRoom(roomSelection);

				cout << string(TheDungeon.CurrentRoom->GetRoomData()["Description"]);
			}


			else {
				cout << "Invalid Exit. \n";
			}

		}
		break;
	default:
		break;

	}


}


void other(vector<string> command) {
	if (command[0] == "debug" or command[0] == "dbug" or command[0] == "d") {
		displayDebugInfo();
	}
};


void displayDebugInfo() {
	cout << "\nCurrent Room: " << TheDungeon.CurrentRoom->GetRoomData()["Name"];
	cout << "\nRooms Loaded: " << TheDungeon.RoomsNumber();
	cout << "\nEntities Loaded: " << TheDungeon.EntitiesNumber();

	cout << "\n\n-----List Of Rooms-----";



	for (int i = 0; i < TheDungeon.RoomsNumber(); i++) {
		cout << "\nRoom " << i << " Name: " << TheDungeon.RoomName(i);
	}

	cout << "\n-----------------------";
	
	cout << "\n\nDirections: " << TheDungeon.CurrentRoom->ConnectedRooms.size();

	cout << "\n-----Directions From Current-----";

	for (int i = 0; i < TheDungeon.CurrentRoom->ConnectedRooms.size(); i++) {
		cout << "\nDirection " << i << " " << TheDungeon.CurrentRoom->ConnectedRooms[i].dirName << ": ";
		cout << TheDungeon.CurrentRoom->ConnectedRooms[i].roomInDir.lock()->GetRoomData()["Name"];
	}
	cout << "\n---------------------------------\n";
}

