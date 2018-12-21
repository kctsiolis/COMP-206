#include <string.h>
#include <stdio.h>

int main(int argc, char* argv[]) {

	if (argc != 2) { //Input checking
		printf("Wrong number of arguments. Ensure to put one link to a Wikipedia article.");
	}
	else {
		FILE* fp;
		fp = fopen(argv[1], "r"); //Opening the Wikipedia page in question
		fseek(fp,0L,SEEK_END); //Seeking to the end of the file to determine its length
		int size = ftell(fp); //Storing the length of the file
		rewind(fp); //Reset to the beginning of file

		char contents[size+2]; //Array that holds the contents of HTML file
		fread(contents,size+1,1,fp); //Reading into the array
		contents[size+1] = '\0'; //Making sure to end the string

		char *scanner = contents; //Pointer to go through contents of file
		const char needle[20] = "<a href=\"/wiki/"; //Array for start of Wikipedia link
		const char title[10] = "title=\""; //Ensuring that the "title" in the link
		const char end[10] = "</a>"; 


		while (*scanner) {
			char *link = strstr(scanner, needle); //Searching for start of Wikipedia link
			if (link) { 
				char *title_checker = strstr(link,title); //Looking for title inside link
				char *end_checker = strstr(link, end); //Checking that title occurs before </a> tag		
				link += strlen(needle); //Moving pointer to title of page
				
				if (title_checker < end_checker) {
					while(*link != '\"') {
						printf("%c", *link); //Print the title one character at a time
						link++;
					}
					printf("\n");
				}
				
				scanner = link + 1;
			}
			else {
				break;
			}

		}
	}

}
