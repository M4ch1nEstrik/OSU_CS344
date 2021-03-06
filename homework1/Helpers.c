#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Helpers.h"


// Create & return new movie node
struct movie* createMovie(char* currLine) {
	struct movie* currMovie = malloc(sizeof(struct movie));
	// Count number of languages.
	currMovie->numLanguages = 0;
	// For use with strtok_r
	char* savePtr;

	/*
	Read and tokenize the line. Expect the following format:
	Title<char*>;Year<int>;Languages[language1<char*>;language2<char*>;...language5<char*>];Rating<double>
	*/

	// Process and store the 1st token, which is the movie title.
	char* token = strtok_r(currLine, ",", &savePtr);
	currMovie->Title = calloc(strlen(token) + 1, sizeof(currMovie->Title)); // Needed if Title is a char*
	strcpy(currMovie->Title, token);

	// Process and store the next token, which is the movie year. 
	token = strtok_r(NULL, ",", &savePtr);
	currMovie->Year = atoi(token);

	// Process and store the next token, which is the string that contains all the languages. Store each language (as a char pointer)as an element in an array.
	token = strtok_r(NULL, ",", &savePtr);
	processMovieLanguages(currMovie, token);

	// // Process and store the last token, which is the movie rating. 
	token = strtok_r(NULL, ",", &savePtr);
	currMovie->Rating = strtod(token, NULL);

	currMovie->next = NULL;

	return currMovie;
};

// Free memory of movies linked list
void freeMovie(struct movie* list) {
	struct movie* temp;
	while (list) {
		temp = list;
		list = list->next;
		free(temp->Title);
		// Free each language
		for (int i = 0; i < MAX_LANGUAGES; ++i) {
			free(temp->Languages[i]);
		}
		free(temp);
	}
};

// Process csv file. No validation performed (correct number of columns, correct format, etc.).
struct movie* processFile(char* filePath, int* numLines)
{
	// Open the specified file for reading only
	FILE* movieFile = fopen(filePath, "r");

	char* currLine = NULL;
	size_t len = 0;
	size_t nread;

	// The head of the linked list
	struct movie* head = NULL;
	// The tail of the linked list
	struct movie* tail = NULL;

	// Check if valid file, and then ignore 1st row (header)
	len = 256;
	if ((nread = getline(&currLine, &len, movieFile)) == -1) {
		printf("Empty file\n");
		return head;
	}
	// Generate linked list of movie nodes by reading the file line by line. Each node holds a record (row) in the csv.
	while ((nread = getline(&currLine, &len, movieFile)) != -1)
	{
		// Create a new movie node corresponding to the current line
		struct movie* newNode = createMovie(currLine);

		// Is this the first node in the linked list?
		if (!head)
		{
			// Set the head and the tail to this new node
			head = newNode;
			tail = newNode;
		}
		else
		{
			// This is not the first node.
			// Add this node to the list and advance the tail.
			tail->next = newNode;
			tail = newNode;
		}
		// Count number of records
		*numLines += 1;
	}
	free(currLine);
	fclose(movieFile);
	return head;
}

// Copy source to destination, with comparator char removed
void filterChar(char* source, char* destination, char comparator)
{
	// Loop through characters, only copy if character != comparator
	while (*source) { // source != '\0'
		if (*source != comparator) {
			*destination++ = *source;
		}
		source += 1;
	}
	*destination = '\0'; // Null terminate
	//char* langSavePtr;
	//destination = strtok_r(source, strcat(comparator, '\0'), &langSavePtr); // Compare needs to be double quotes "" (size 2, char + \0), not ''. 
	//destination[strlen(destination)] = '\0';
	//strcpy(currMovie->Languages[languageIter], lang);
}

// Used to validate user integer input. Assumes valid integer input.
int validateInputInt(const char* menu, const int lbound, const int ubound) {
	int retVal;
	printf("%s", menu);
	int isValid = scanf("%d", &retVal);
	// Reprompt user until valid integer is entered.
	// Doesn't catch number then character such as 3a, only reads 3 and discards a
	while (isValid == 0 || retVal < lbound || retVal > ubound) {
		printf("\nYou entered an incorrect choice. Try again.\n\n");
		flushStdin();
		printf("%s", menu);
		isValid = scanf("%d", &retVal);
	};
	flushStdin();
	return retVal;
};

// Flush STDIN buffer
void flushStdin(void) {
	int ch;
	while (((ch = getchar()) != '\n') && (ch != EOF));
}

// Creates a linked list of <year, <title, rating>> nodes from a list of movie nodes. All years are unique with the highest rating stored for that year.
struct keyValues* createKeysValueList(struct movie* list) {
	// Create first node in list, increment movie pointer to next node
	struct keyValues* head = createKeysValue(list);
	struct keyValues* tail = head;
	struct movie* movieList = list->next;

	// Loop through each remaining movie node and create and add <year, <title, rating>> node to list if unique year.
	// Else swap info for existing node if new rating is higher.
	while (movieList) {
		// Try to find matching/existing movie year, and replace with highest rating if needed
		while (tail->next) {
			// If node with existing year exists, set to higher rating
			if (movieList->Year == tail->year) {
				if (movieList->Rating > tail->titleRating.rating) {
					// Ensure adequate memory in source, then copy
					tail->titleRating.title = realloc(tail->titleRating.title, strlen(movieList->Title));
					strcpy(tail->titleRating.title, movieList->Title);
					tail->titleRating.rating = movieList->Rating;
				}
				break;
			}
			tail = tail->next;
		}
		// Loop above stops without checking last node (since tail->next == NULL). Need to check last node in tail list.
		if (movieList->Year == tail->year) {
			if (movieList->Rating > tail->titleRating.rating) {
				// Ensure adequate memory in source, then copy
				tail->titleRating.title = realloc(tail->titleRating.title, strlen(movieList->Title));
				strcpy(tail->titleRating.title, movieList->Title);
				tail->titleRating.rating = movieList->Rating;
			}
		}
		else {
			// Create new node and add to list since current year does not exist
			struct keyValues* newNode = createKeysValue(movieList);
			tail->next = newNode;
		}
		tail = head; // rest tail to the beginning
		movieList = movieList->next; //move to next movie node
	}
	return head;
}

// Creates a <year, <title, rating>> struct from movie node
struct keyValues* createKeysValue(struct movie* list) {
	struct keyValues* temp = malloc(sizeof(struct keyValues));
	temp->titleRating.title = malloc(strlen(list->Title));
	strcpy(temp->titleRating.title, list->Title);
	temp->titleRating.rating = list->Rating;
	temp->year = list->Year;
	temp->next = NULL;
	return temp;
}

// Display title and highest rating for each year
void printKeysValue(struct keyValues* list) {
	struct keyValues* head = list;
	while (head) {
		printf("%d %0.1f %s\n", head->year, head->titleRating.rating, head->titleRating.title);
		head = head->next;
	}
}

// Frees the linked list of nodes containing <year, <title, rating>>
void freeKeysValue(struct keyValues* list) {
	struct keyValues* temp;
	while ((temp = list) != NULL) {
		list = list->next;
		free(temp->titleRating.title);
		free(temp);
	}
	list = NULL;
};

// Print all Movies with given language
// Find caseChoice (language) in file. If not found, printf:
// No input validation needed
// Exact match needed i.e. 'English' != 'english'
void printMoviesByLanguage(struct movie* list, char* lang) {
	struct movie* temp = list;
	int exists = 0;
	while (temp != NULL) {
		for (int i = 0; i < temp->numLanguages; ++i) {
			if (strcmp(temp->Languages[i], lang) == 0) {
				printf("%d %s\n", temp->Year, temp->Title);
				exists = 1;
				break;
			}
		}
		temp = temp->next;
	};
	if (!exists) {
		printf("No data about movies released in %s\n", lang);
	}
}

// Process languages string into separate language elements in an array.
void processMovieLanguages(struct movie* currMovie, char* token) {
	int languageIter = 0;
	char* langPtr;
	// Tokenize language array, which is in the following format: [language1<char*>;language2<char*>;...language5<char*>]
	char* languageToken = strtok_r(token, ";", &langPtr);
	// Loop through each language, from 1 - 5 languages, and handle each case:
	while (languageToken != NULL && languageIter < MAX_LANGUAGES) {
		// Create character array for this language/
		currMovie->Languages[languageIter] = calloc(strlen(languageToken) + 1, sizeof(currMovie->Languages[languageIter]));
		int s = strlen(languageToken) - 1;
		// If one language, format is [some_language]. Filter out brackets
		if (languageToken[0] == '[' && languageToken[s] == ']') {
			filterChar(languageToken, currMovie->Languages[languageIter], '[');
			filterChar(currMovie->Languages[languageIter], currMovie->Languages[languageIter], ']');
		}
		// 1st language being processed.
		else if (languageToken[0] == '[') {
			filterChar(languageToken, currMovie->Languages[languageIter], '[');
		}
		// Last language being processed.
		else if (languageToken[s] == ']') {
			filterChar(languageToken, currMovie->Languages[languageIter], ']');
		}
		// All other cases (processing languages in-between)
		else {
			strcpy(currMovie->Languages[languageIter], languageToken);
		}
		languageToken = strtok_r(NULL, ";", &langPtr);
		languageIter += 1;
	}
	currMovie->numLanguages = languageIter;
}