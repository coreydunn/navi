/*
 * Navigate filesystem with ncurses and CLI
 * */
#include<curses.h>
#include<dirent.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<string>
#include<vector>
#include<algorithm>

bool isdir(std::string p);
std::string trim(std::string s);
std::string up(std::string p);

int main(int argc,char**argv,char**env)
{
	std::string pwd="";
	std::string wd="";
	int running=true;

	// Get pwd
	for(size_t i=0;env[i];++i)
		if(strncmp(env[i],"PWD",3)==0)
			pwd=strchr(env[i],'=')+1;

	// Set wd
	if(argc>1)
		wd=argv[1];
	else
		wd=trim(pwd+"/"+wd);

	printf("up: '%s'\n",up(wd).c_str());

	// Init curses
	initscr();
	raw();
	noecho();

	// Main loop
	while(running)
	{
		DIR*dirp;
		struct dirent*dir;
		std::vector<std::string>v;
		std::string info;
		int choosing;
		int choice;
		size_t numeric_modifier;

		// Init
		choosing=true;
		choice=0;
		numeric_modifier=1;
		dir=NULL;
		dirp=NULL;

		// Open pwd
		dirp=opendir(wd.c_str());
		if(!dirp)
		{
			fprintf(stderr,"error: cannot open '%s'\n",wd.c_str());
			break;
		}

		while(dir=readdir(dirp))
		{
			std::string dname=dir->d_name;

			// Exclude current directory
			if(strcmp(dname.c_str(),".")!=0)
			{
				if(isdir(wd+'/'+dname))
					dname+="/";
				v.push_back(dname);
			}
		}
		std::sort(v.begin(),v.end());

		// Render
		clear();
		move(0,0);
		printw("pwd: %s\n",wd.c_str());
		for(size_t i=0;i<v.size();++i)
			printw("%s\n",v[i].c_str());
		move(1,0);
		refresh();

		while(choosing)
		{
			char c=getchar();

			switch(c)
			{

				case 'q':
					choosing=false;
					running=false;
					puts("user quit");
					break;

				case 'j':
					if(choice<v.size()-1)
					{
						choice+=numeric_modifier;
						if(choice>v.size()-1)
							choice=v.size()-1;
						numeric_modifier=1;
					}
					break;

				case 'k':
					if(choice>0)
					{
						choice-=numeric_modifier;
						if(choice<0)choice=0;
						numeric_modifier=1;
					}
					break;

				case '/':
					{
						char b[1024]={0};
						bool found=false;

						echo();
						mvprintw(0,30,"search: ");
						mvscanw(0,38,"%s",b);
						noecho();
						for(size_t i=0;i<v.size();++i)
							if(strncmp(v[i].c_str(),b,2)==0)
							{
								choice=i;
								found=true;
							}
						if(!found)info=std::string("error: cannot find '")+b+"'";
					}
					break;

				case 'h':
					{
						choosing=false;
						wd=up(wd);
						info=std::string("wd: ")+wd;
					}
					break;

				case 'l':
					{
						std::string newwd=wd+"/"+v[choice];

						if(isdir(newwd))
						{
							info="cd: "+newwd;
							choosing=false;
							if(strncmp(v[choice].c_str(),"..",2)==0)
								wd=up(wd);
							else
								wd=trim(newwd);
						}
						else
							info="error: "+newwd+" is not a directory";
					}
					break;

				case '1':case '2':case '3':case '4':case '5':
				case '6':case '7':case '8':case '9':
					numeric_modifier=c-'0';
					break;
			}

			mvprintw(0,30,"'%s'        ",info.c_str());
			if(numeric_modifier!=1)
				mvprintw(getmaxy(stdscr)-1,0,"%lu",numeric_modifier);
			else
				mvprintw(getmaxy(stdscr)-1,0,"     ");
			move(choice+1,0);
			refresh();
		}
	}

	endwin();
	printf("running: %lu\n",running);
	printf("wd: '%s'\n",wd.c_str());
}


bool isdir(std::string p)
{
	struct stat buf;
	stat(p.c_str(),&buf);
	return S_ISDIR(buf.st_mode);
}

std::string trim(std::string s)
{
	std::string::size_type pos=s.find_last_of("/");
	if(pos!=std::string::npos)
		s.erase(pos);
	return s;
}

std::string up(std::string p)
{
	size_t li=p.rfind('/');
	std::string last=p.substr(p.rfind('/'));

	if(last.size()==1)
		last=p.substr(p.rfind('/',p.rfind('/',li-1)-1));

	return (p.substr(p.find('/'))==last) ? std::string("/") : p.substr(0,li);
}
