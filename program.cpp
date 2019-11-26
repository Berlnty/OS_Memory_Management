#include <iostream>
#include <vector>
#include <map>
#include<utility>
#include <cmath>
#include <queue>
#include <sstream>
#include <fstream>
#include <string>

using namespace std;

struct process {

	string id;
	int arrival_time;
	int remaning_time;
	int duration;
	int times_to_be_queed = 0;
	int start_address;
	int end_address;
	int memSize;
	int memSize_rounded;
	int index_block;

};
process current_process;


vector<vector<pair<int, int> > >Memory;   /*
										  first -> state
										  second -> parent
										  index 0 (1024) -> max size  , index 7 (8) -> min size as there is overhead 6B
										  0 -> hole available
										  1 -> hole occupied
										  2 -> not in the memory
										  */
map<int, int>size_indix;
map<int, int>indix_size;
queue<process>ready_queue;
vector<process> processes;
vector<string>names;

int quantum, switch_s, read_message_index, Clock;
bool is_clockInc = false;
ofstream outfile("out.txt");



int Next_pow2(int x);
void Initialize_Mem();
void map_list();
bool allocate(int list_num, int &start_add, int &indx_me);
void merge_buddies(int& x, int& y);
void deallocate(process p);
void read_msg();
void print_queue();
void ALGO_RR();
void reading_input();



int main()
{

	read_message_index = 0; Clock = 0;
	current_process.remaning_time = -1;


	Initialize_Mem();
	map_list();
	reading_input();
	
	// output in log file
	

	while (!ready_queue.empty() || (read_message_index<processes.size())) {

		is_clockInc = false;
		read_msg();
		ALGO_RR();

		if (is_clockInc == false)
		{
			Clock++;
			cout << "INC clock: " << endl;
		}

		cout << "clock: " << Clock << endl;

	}


	outfile.close();
	return 0;
}

int Next_pow2(int x)
{
	return pow(2, ceil(log(x) / log(2)));
}

void Initialize_Mem()
{
	/* all list of holes are initially empty except the max one (1024) */

	vector<pair<int, int>>Memsize_1024, Memsize_512, Memsize_256, Memsize_128, Memsize_64, Memsize_32, Memsize_16, Memsize_8;
	Memsize_1024.push_back(make_pair(0, -1));
	Memory.push_back(Memsize_1024);
	Memory.push_back(Memsize_512);
	Memory.push_back(Memsize_256);
	Memory.push_back(Memsize_128);
	Memory.push_back(Memsize_64);
	Memory.push_back(Memsize_32);
	Memory.push_back(Memsize_16);
	Memory.push_back(Memsize_8);
}

void map_list()
{
	size_indix[1024] = 0;
	indix_size[0] = 1024;

	size_indix[512] = 1;
	indix_size[1] = 512;

	size_indix[256] = 2;
	indix_size[2] = 256;

	size_indix[128] = 3;
	indix_size[3] = 128;

	size_indix[64] = 4;
	indix_size[4] = 64;

	size_indix[32] = 5;
	indix_size[5] = 32;

	size_indix[16] = 6;
	indix_size[6] = 16;

	size_indix[8] = 7;
	indix_size[7] = 8;
}

bool allocate(int list_num, int &start_add, int &indx_me)      //return -1 if cannot allocate the process
{

	if (list_num == 0)   //max size   //base case
	{
		for (int i = 0; i<Memory[list_num].size(); i++)
		{

			if (Memory[list_num][i].first == 0)  //if empty block
			{
				Memory[list_num][i].first = 2;     //split it into two halves
				Memory[list_num][i].second = -1;   //has no parent
				indx_me = 0;

				return true;
			}
		}
		return false;     //no space for the process (put it into the end of the queue)
	}

	for (int i = 0; i<Memory[list_num].size(); i++)
	{

		if (Memory[list_num][i].first == 0)  //if empty block
		{
			Memory[list_num][i].first = 1;   //reserve it to the process

											 //change start and end address
			start_add = Memory[list_num][i].second*indix_size[list_num - 1];
			start_add += (indix_size[list_num] * (i % 2));

			indx_me = i;

			return true;
		}
	}


	if (allocate(list_num - 1, start_add, indx_me))
	{

		Memory[list_num - 1][indx_me].first = 2;
		pair<int, int> first_buddy = make_pair(1, indx_me);   //reserve it to the process
		pair<int, int> second_buddy = make_pair(0, indx_me);

		Memory[list_num].push_back(first_buddy);
		Memory[list_num].push_back(second_buddy);

		//change start and end address
		int Blocks_num = Memory[list_num].size();

		//	start_add += indix_size[list_num] * (Blocks_num - 2);

		indx_me = Memory[list_num].size() - 2;
		return true;
	}

	return false;      //put the process at the end of the queue
}
void merge_buddies(int& x, int& y)
{
	Memory[x][y].first = Memory[x][y + 1].first = 0;   //not available
	Memory[x - 1][Memory[x][y].second].first = 0;
	//Memory[x][y].second = Memory[x][y + 1].second = Memory[x - 1][Memory[x][y].second].second;
	int xd = x;
	int yd = y;
	y = Memory[x][y].second;
	x -= 1;

	Memory[xd].erase(Memory[xd].begin() + yd);
	Memory[xd].erase(Memory[xd].begin() + yd);

}
void deallocate(process p)
{
	int i = size_indix[p.end_address - p.start_address + 1];
	int j = p.index_block;

	cout << i << " " << j << endl;
	Memory[i][j].first = 0;

	bool flag = true;

	while (flag)
	{

		flag = false;

		if ((i != 0) && (j % 2 == 0) && (Memory[i][j + 1].first == 0))
		{
			flag = true;

			merge_buddies(i, j);
		}

		else if ((j % 2 == 1) && Memory[i][j - 1].first == 0) {
			flag = true;
			int y = j - 1;
			merge_buddies(i, y);
			j = y;
		}
		if (i == 0)
		{
			flag = false;
			Memory[0][0].first = 0;
		}
	}
}

void read_msg() {

	for (read_message_index; read_message_index<processes.size();) {
		int u;
		u = (Clock >= switch_s) ? Clock - switch_s : Clock;

		if (processes[read_message_index].arrival_time <= u) {
			ready_queue.push(processes[read_message_index]);
			names.push_back(processes[read_message_index].id);
			cout << "push in ready queue --> process id: " << processes[read_message_index].id << " at time" << Clock << endl;
			read_message_index++;
		}
		else
			break;
	}

	return;
}

void print_queue()
{
	outfile << "Queue:";
	for (int i = 0; i<names.size(); ++i)
	{
		outfile << " " + names[i];

	}
	outfile << endl;
}

void ALGO_RR() {

	if (current_process.remaning_time>0)
	{
		ready_queue.push(current_process);
		names.push_back(current_process.id);
		cout << "push current back in queue with remaning time " << current_process.remaning_time << " id " << current_process.id << " at clock :" << Clock << endl;
	}

	if (!ready_queue.empty())
	{
		current_process = ready_queue.front();
		ready_queue.pop();
		print_queue();
		names.erase(names.begin());

		if (current_process.remaning_time == current_process.duration)   //allocate
		{

			int add = 0;
			int indX_me = 0;
			int indx = size_indix[current_process.memSize_rounded];


			if (allocate(indx, add, indX_me))
			{
				current_process.index_block = indX_me;
				current_process.start_address = add;
				current_process.end_address = add + current_process.memSize_rounded - 1;
				
				outfile << "Executing process " << current_process.id + " : " << "started at " << Clock;
				is_clockInc = true;
				Clock += min(quantum, current_process.remaning_time);
				current_process.remaning_time -= min(quantum, current_process.remaning_time);
				string h = (current_process.remaning_time > 0) ? ", stopped at " : ", finished at ";
				string j = (current_process.remaning_time > 0) ? to_string(current_process.remaning_time) + " remaining" : "";
				outfile << h << Clock << ", " << j << " memory starts at " << current_process.start_address << " and ends at " << current_process.end_address << endl;



				if ((ready_queue.empty() && read_message_index == processes.size() && current_process.remaning_time == 0) || (ready_queue.empty() && current_process.remaning_time == 0 && read_message_index != processes.size() &&processes[read_message_index].arrival_time>= Clock))
					Clock += 0;
				else {
					outfile << "Process switching : started at " << Clock << ", finished at ";
					Clock += switch_s;
					outfile << Clock << endl;
				}

			}

			else if (current_process.times_to_be_queed <= 5)
			{
				current_process.times_to_be_queed++;
				ready_queue.push(current_process);

			}

			else if (current_process.times_to_be_queed > 5)
			{
				cout << "Sorry No space in memory for process of id: " << current_process.id << endl;
			}

		}

		else    //continue the process
		{

			cout << " continue " << current_process.remaning_time << " id " << current_process.id << " clock : " << Clock << endl;
			is_clockInc = true;



			outfile << "Executing process " << current_process.id + " : " << "started at " << Clock;
			Clock += min(quantum, current_process.remaning_time);
			current_process.remaning_time -= min(quantum, current_process.remaning_time);
			string h = (current_process.remaning_time > 0) ? ", stopped at " : ", finished at ";
			string j = (current_process.remaning_time > 0) ? to_string(current_process.remaning_time) + " remaining" : "";
			outfile << h << Clock << ", " << j << " memory starts at " << current_process.start_address << " and ends at " << current_process.end_address << endl;

			if ((ready_queue.empty() && read_message_index == processes.size() && current_process.remaning_time == 0) || (ready_queue.empty() && current_process.remaning_time == 0 && read_message_index != processes.size() && processes[read_message_index].arrival_time >= Clock))
				Clock += 0;
			else
			{
				outfile << "Process switching : started at " << Clock << ", finished at ";
				Clock += switch_s;
				outfile << Clock << endl;
			}
		}


		if (current_process.remaning_time == 0)
		{

			deallocate(current_process);

		}



	}
	return;
}

void reading_input()
{
	string line;
	ifstream myfile("t1.txt");
	if (myfile.is_open())
	{

		getline(myfile, line);
		stringstream s0;
		string temp;
		s0 << line;
		s0 >> temp >> quantum;


		getline(myfile, line);
		stringstream s1;
		s1 << line;
		s1 >> temp >> switch_s;

		getline(myfile, line);
		while (getline(myfile, line))
		{
			stringstream s2;
			process process;
			s2 << line;
			s2 >> process.id >> process.duration >> process.arrival_time >> process.memSize;
			process.remaning_time = process.duration;
			process.memSize_rounded = Next_pow2(process.memSize + 6);
			processes.push_back(process);
		}
		cout << quantum << " " << switch_s << endl;
		myfile.close();
	}

}