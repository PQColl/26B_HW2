/*********************************************************************************
** CIS 26B                                                            Winter, 2018
** Advanced C
******************
**
** Homework 2: 
**        A Circularly Doubly Linked List of Stacks with a Sentinel Node
**
**********************************************************************************

   This program reads a file containing stock information and stores the stock
   symbol in a circularly doubly linked list, and the quotes on a stack assigned to
   each stocks node. The list is then searchable and printable depending on the user
   input.

   Save the output as a comment at the end of the program!
**********************************************************
**
**  Written By: Patrick Collins          // <-- write your name here
**
**  Date: 2/8/2018                 // <-- write the date here
***************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _MSC_VER
#include <crtdbg.h>  // needed to check for memory leaks (Windows only!)
#endif

#define DUMMY_TRAILER '\177'
#define FLUSH while(getchar() != '\n')

typedef struct quote QUOTE;
struct quote
{
	double value;
	QUOTE *next;
};

typedef struct node NODE;
struct node
{
	char symbol[5];
	int size;
	QUOTE *quotes;
	NODE  *forw;
	NODE  *back;
};

NODE *init_list(void);
void  createList(NODE *list);
char *getFilename();
int   insert(NODE *list, char *symbol, double value);

QUOTE *push(QUOTE *quotes, float value);
QUOTE *pop(QUOTE **quotes);

void displayManager(NODE *list);
void printDisplayHeader();
void printSymbolList(NODE *list);
void printListForw(NODE *list, char *beg, char *end);
void printListBack(NODE *list, char *beg, char *end);

NODE *findNode(NODE *list, const char *target);

void searchManager(NODE *list);
void printSearchHeader();
char *getSymbol(char *symbol);
int getQuoteAmount(const char *symbol);
void printQuotes(NODE *stock, int amount);

void  traverse_forw(NODE *list);
void  traverse_back(NODE *list);

NODE *freeList(NODE *list);
char *allocateString(char *inString);

int main( void )
{
	NODE *list;

	list = init_list();
	createList(list);

	traverse_forw(list);
	traverse_back(list);

	displayManager(list);

	searchManager(list);

	list = freeList(list);
    #ifdef _MSC_VER
        printf( _CrtDumpMemoryLeaks() ? "Memory Leak\n" : "No Memory Leak\n");
    #endif

	system("pause");
    return 0;
}

/****************** init_list ********************
 *	Initialization of a circularly doubly-linked
 *	list with one sentinel node
 *		PRE:  none
 *		POST: initialized circularly doubly-linked
 *			  list with sentinel node.
 *************************************************/
NODE *init_list(void)
{
	NODE *list;

	// allocate the sentinel node
	list = (NODE *)malloc(sizeof(NODE));
	if (!list)
	{
		printf("Error in init_list!\n");
		exit(1);
	}
	
	list->symbol[0] = DUMMY_TRAILER;
	list->symbol[1] = '\0';
	
	list->forw = list;
	list->back = list;

	return list;
}

/************** getFilename *****************
* this function generates the name of input
* file based on user input.
*		PRE: none
*		POST: dynamically allocated string
********************************************/
char *getFilename()
{
	char tmpFile[100], *filename;
	printf("Please enter the name of the file you wish to open: ");
	fgets(tmpFile, sizeof(tmpFile), stdin);

	if (strlen(tmpFile) == 1)
		filename = allocateString("stocks.txt");
	else 
	{
		char *ptr = strchr(tmpFile, '\n');
		*ptr = '\0';
		filename = allocateString(tmpFile);
	}
	return filename;
}

void createList(NODE *list)
{
	char *filename, symbol[5];
	double value;
	FILE *fpStocks;

	filename = getFilename();
	fpStocks = fopen(filename, "r");

	if (!fpStocks)
		printf("ERROR opening %s\n", filename), exit(101);

	while ((fscanf(fpStocks, "%4s %lf", symbol, &value)) != EOF)
		insert(list, symbol, value);

	fclose(fpStocks);
	free(filename);
}
/******************* insert ***********************
 *	Insert a node in a sorted circularly
 *	doubly-linked list with one sentinel node
 *		PRE: list - pointer to first node in list
 *			 symbol - c-string of symbol to insert
 *			 value - quote value
 *		POST: 1 - if duplicate
 *			  0 - otherwise
 **************************************************/
int insert(NODE *list, char *symbol, double value)
{
	NODE *curr = list->forw;
	NODE *prev = list;
	NODE *pnew;

	int   duplicate = 1;

	while (strcmp(symbol, curr->symbol) > 0)
	{
		prev = curr;
		curr = curr->forw;
	}
	if (strcmp(symbol, curr->symbol))
	{
		duplicate = 0; // not a duplicate
		pnew = (NODE *)malloc(sizeof(NODE));
		if (!pnew)
		{
			printf("Fatal memory allocation error in insert!\n");
			exit(3);
		}

		//initialize stack of quotes when inserting the stock into the list
		strcpy(pnew->symbol, symbol);
		pnew->quotes = NULL;
		pnew->quotes = push(pnew->quotes, value);
		pnew->size = 1;
		pnew->forw = curr;
		pnew->back = prev;
		prev->forw = pnew;
		curr->back = pnew;
	}
	//if already in list, add the quote to its stack
	else
	{
		curr->quotes = push(curr->quotes, value);
		curr->size++;
	}
	
	
	return duplicate;
}

/********************* push ***********************
 * Stack Insert: insert in the beginning
 *		PRE: stack - pointer to top of stack
 *			 value - quote to push
 *		POST: new quote pushed to top of stack,
 *			  pointer to top of stack returned.
 ***************************************************/
QUOTE *push(QUOTE *stack, float value)
{
	QUOTE *pnew;

	pnew = (QUOTE *)malloc(sizeof(QUOTE));
	if (!pnew)
	{
		printf("... error in push!\n");
		exit(1);
	}
	pnew->value = value;
	pnew->next = stack;
	stack = pnew;

	return stack;
}


/********************** pop **********************
 * Stack Delete: delete the first node
 *		PRE: stack - double level pointer to top
 *					 of stack
 *		POST: top of stack popped and quote returned
 *************************************************/
QUOTE *pop(QUOTE **stack)
{
	QUOTE *first;

	if (*stack == NULL) return NULL;
	first = *stack;
	*stack = (*stack)->next;
	first->next = NULL;

	return first;
}

/******************* displayManager ***********************
 * Handles the user controlled display prompts. If input is
 * valid prints the quotes forward and backward.
 *
 *		PRE: list - pointer to the circularly doubly linked
 *					list
 *		POST: user input read and processed, results displayed
 **********************************************************/

void displayManager(NODE *list)
{
	printDisplayHeader();
	printSymbolList(list);

	char *delim = "ZZZZ";
	char s1[5], s2[5];


	while (strcmp(s1, delim) && strcmp(s2, delim))
	{
		scanf("%4s %4s", s1, s2);
		FLUSH;
		if (strcmp(s1, delim) && strcmp(s2, delim))
		{
			if (!findNode(list, s1))
				printf("%s - not found\n", s1);
			else if (!findNode(list, s2))
				printf("%s - not found\n", s2);
			else
			{
				printListForw(list, s1, s2);
				printListBack(list, s1, s2);
			}
			putchar('\n');
		}
	}

}

/******************* printListForw ***********************
*	prints the list from the target beginning to end
*   going forward.
*		PRE: list - pointer to circularly doubly linked list
*			 beg  - target string for the beginning of list
*			 end  - target string for the end of list 
*		POST: list is printed from range beginning to end
*			  forwards
**********************************************************/
void printListForw( NODE *list, char *beg, char *end)
{
	list = list->forw;
	
	while (strcmp(list->symbol, beg))
		list = list->forw;

	printf("%s to %s: ", beg, end);

	while (strcmp(list->symbol, end))
	{
		if (list->symbol[0] != DUMMY_TRAILER)
			printf("[%s: %.2f] ", list->symbol, list->quotes->value);
		list = list->forw;
	}
	printf("[%s: %.2f]\n", list->symbol, list->quotes->value);
}

/******************* printListBack ***********************
*	prints the list from the target beginning to end
*   going backwards.
*		PRE: list - pointer to circularly doubly linked list
*			 beg  - target string for the beginning of list
*			 end  - target string for the end of list
*		POST: list is printed from range beginning to end
*			  backwards
**********************************************************/
void printListBack(NODE *list, char *beg, char *end)
{
	list = list->back;

	while (strcmp(list->symbol, beg))
		list = list->back;

	printf("%s to %s: ", beg, end);

	while (strcmp(list->symbol, end))
	{
		if (list->symbol[0] != DUMMY_TRAILER)
			printf("[%s: %.2f] ", list->symbol, list->quotes->value);
		list = list->back;
	}
	printf("[%s: %.2f]\n", list->symbol, list->quotes->value);
}
/***************** printDisplayHeader *********************
*	prints the information header for the display manager
*		PRE: none
*		POST: display manager header displayed
**********************************************************/
void printDisplayHeader()
{
	printf("\n --------------- Display Manager -----------------\n"
		   "  Enter the symbols of two companies to print the \n"
	       "  list of stocks between them forward and backward.\n"
		   "  See below for list of stock symbols.\n\n"
		   "  Enter format: \"SYMBOL1 SYMBOL2\"\n"
		   "                use ZZZZ as a symbol to exit.\n\n");
}
/******************** printSymbolList *********************
* prints the entire list of stock symbols from the start
* to end of passed circularly doubly linked list
*		PRE: list - pointer to circularly doubly linked list
*		POST: stock symbols in list displayed
**********************************************************/
void printSymbolList(NODE *list)
{
	list = list->forw;

	while (list->symbol[0] != DUMMY_TRAILER)
	{
		printf("%s ", list->symbol);
		list = list->forw;
	}
	putchar('\n');
	putchar('\n');
}

/****************** searchManager *************************
* manages the searching process through the list based on
* user input.
*		PRE: list - pointer to circularly doubly linked list
*		POST: the list is searched based on user input.
**********************************************************/
void searchManager(NODE *list)
{
	char symbol[5];
	int amount;
	NODE *tStock;
	printSearchHeader();

	while (strcmp(strcpy(symbol, getSymbol(symbol)), "QUIT"))
	{
		if ((tStock = findNode(list, symbol)))
		{
			amount = getQuoteAmount(symbol);
			printQuotes(tStock, amount);
		}
		else
			printf("%s not found.\n", symbol);
	}
}

/***************** printSearchHeader *********************
* displays the header and instructions for searching the 
* list.
*		PRE: none
*		POST: header and instructions displayed
**********************************************************/
void printSearchHeader()
{
	printf("\n --------------- Search Manager -----------------\n"
			"    Enter the stock symbol you wish to search for.\n"
			"    If the symbol is in the list, you will be     \n"
			"    prompted for the number of quotes of that     \n"
			"    symbol you want to see. When you wish to quit \n"
		    "	 enter QUIT as the stock symbol.\n\n"			  );
}

/******************** getSymbol **************************
*	prompts the user for a symbol and stores it in a
*	cstring allocated on the heap.
*		PRE: symbol - string of target symbol
*		POST: symbol - user input
**********************************************************/
char *getSymbol(char *symbol)
{

	printf("Please enter the symbol you want to search for: ");
	scanf("%4s", symbol);
	FLUSH;

	return symbol;
}
/******************* getQuoteAmount *********************
* prompts the user for the amount of quotes they wish to
* view
*		PRE: symbol - string of the stock symbol
*		POST: amount - int of amount of quotes
**********************************************************/
int getQuoteAmount(const char *symbol)
{
	int amount;

	printf("How many quotes of %s would you like to view? ", symbol);
	scanf("%d", &amount);
	
	return amount;
}
/******************** printQuotes ************************
* prints the quotes for the stock symbol based on user
* input. If the user input an invalid quote amount
* the function displays an error and returns to the search
* manager.
*		PRE: stock - pointer to circularly doubly linked list
*			 amount - count of quotes to print
*		POST: amount is valid - quotes displayed
*			  otherwise - error displayed
**********************************************************/
void printQuotes(NODE *stock, int amount)
{
	if (amount > stock->size)
	{
		printf("Invalid size passed\n");
		return;
	}
	
	printf("%s: ", stock->symbol);

	QUOTE *quote = stock->quotes;
	while (amount > 0)
	{
		printf("%.2f ", quote->value);
		quote = quote->next;
		amount--;
	}
	putchar('\n');

}
/********************** findNode *************************
* finds the node in the list based on the passed symbol
* string. Returns a boolean value if the node is found
* and a pointer is passed back to the node through the
* loc value. If the pointer location is not needed, NULL
* can be passed for the loc value; doing so will only check
* if the list contains the desired node.
*		PRE: list - pointer to circularly doubly linked list
*			 target - string of stock symbol to find
*		POST: targetNode - pointer to node if found
*						 - otherwise NULL
**********************************************************/
NODE *findNode(NODE *list, const char *target)
{
	NODE *targetNode = NULL;
	list = list->forw;

	while (list->symbol[0] != DUMMY_TRAILER && !targetNode)
	{
		if (!strcmp(list->symbol, target))
			targetNode = list;
		else
			list = list->forw;
	}

	return targetNode;
}

/***************** traverse_forw *********************
 * Traverses forward a circularly doubly-linked
 * list with one sentinel node to print out the
 * contents of each node
 *		PRE: list - pointer to list
 *		POST: list contents printed forward
 *****************************************************/
void traverse_forw( NODE *list)
{	
	putchar('\n');
	list = list->forw; // skip the dummy node
	while (list->symbol[0] != DUMMY_TRAILER)
	{
		printf("%-4s %.2f\n", list->symbol, list->quotes->value);
		list = list->forw;
	}
	return;
}

/***************** traverse_back *********************
* Traverses backwards a circularly doubly-linked
* list with one sentinel node to print out the
* contents of each node
*		PRE: list - pointer to list
*		POST: list contents printed backwards
*****************************************************/
void traverse_back(NODE *list)
{
	putchar('\n');
	list = list->back; // skip the dummy node
	while (list->symbol[0] != DUMMY_TRAILER)
	{
		printf("%-4s %.2f\n", list->symbol, list->quotes->value);
		list = list->back;
	}
	return;
}
/********************** freeList ************************
* frees the passed circularly doubly linked list
*	PRE: list - pointer to list to be freed
*	POST: NULL
**********************************************************/
NODE *freeList(NODE *list)
{	
	NODE *next;
	QUOTE *top;
	list = list->forw;

	while (list->symbol[0] != DUMMY_TRAILER)
	{
		next = list->forw;
		while ((top = pop(&(list->quotes))))
			free(top);
		free(list);
		list = next;
	}

	free(list);

	return NULL;
}

/**************** allocateString ******************
* dynamically allocates a string
*		PRE: char *inString - original string
*		POST: outstring - dynamically allocated string
**************************************************/
char *allocateString(char *inString)
{
	char *outString;
	int   stringSize;

	stringSize = strlen(inString) + 1;
	outString = (char *)calloc(stringSize, sizeof(char));
	if (outString == NULL)
		printf("ERROR, not enough memory!!!\a\n"), exit(104);
	strcpy(outString, inString);

	return outString;
}

/************************** !!!! OUTPUT !!!!   ***********************************/
/*
Please enter the name of the file you wish to open: stocks.txt

AAPL 119.73
AMZN 810.11
CSCO 30.04
EBAY 30.64
FB   127.85
GOOG 804.46
INTC 23.94
MSFT 32.60
NFLX 139.39
ORCL 32.78
ZNGA 2.64

ZNGA 2.64
ORCL 32.78
NFLX 139.39
MSFT 32.60
INTC 23.94
GOOG 804.46
FB   127.85
EBAY 30.64
CSCO 30.04
AMZN 810.11
AAPL 119.73

--------------- Display Manager -----------------
Enter the symbols of two companies to print the
list of stocks between them forward and backward.
See below for list of stock symbols.

Enter format: "SYMBOL1 SYMBOL2"
use ZZZZ as a symbol to exit.

AAPL AMZN CSCO EBAY FB GOOG INTC MSFT NFLX ORCL ZNGA

GOOG FB
GOOG to FB: [GOOG: 804.46] [INTC: 23.94] [MSFT: 32.60] [NFLX: 139.39] [ORCL: 32.78] [ZNGA: 2.64] [AAPL: 119.73] [AMZN: 810.11] [CSCO: 30.04] [EBAY: 30.64] [FB: 127.85]
GOOG to FB: [GOOG: 804.46] [FB: 127.85]

AAPL ORCL
AAPL to ORCL: [AAPL: 119.73] [AMZN: 810.11] [CSCO: 30.04] [EBAY: 30.64] [FB: 127.85] [GOOG: 804.46] [INTC: 23.94] [MSFT: 32.60] [NFLX: 139.39] [ORCL: 32.78]
AAPL to ORCL: [AAPL: 119.73] [ZNGA: 2.64] [ORCL: 32.78]

CSGO INTS
CSGO - not found

CSCO INTS
INTS - not found

CSCO INTC
CSCO to INTC: [CSCO: 30.04] [EBAY: 30.64] [FB: 127.85] [GOOG: 804.46] [INTC: 23.94]
CSCO to INTC: [CSCO: 30.04] [AMZN: 810.11] [AAPL: 119.73] [ZNGA: 2.64] [ORCL: 32.78] [NFLX: 139.39] [MSFT: 32.60] [INTC: 23.94]

AAPL ZNGA
AAPL to ZNGA: [AAPL: 119.73] [AMZN: 810.11] [CSCO: 30.04] [EBAY: 30.64] [FB: 127.85] [GOOG: 804.46] [INTC: 23.94] [MSFT: 32.60] [NFLX: 139.39] [ORCL: 32.78] [ZNGA: 2.64]
AAPL to ZNGA: [AAPL: 119.73] [ZNGA: 2.64]

ZZZZ CSCO

--------------- Search Manager -----------------
Enter the stock symbol you wish to search for.
If the symbol is in the list, you will be
prompted for the number of quotes of that
symbol you want to see. When you wish to quit
enter QUIT as the stock symbol.

Please enter the symbol you want to search for: CSCO
How many quotes of CSCO would you like to view? 4
CSCO: 30.04 20.38 20.88 20.98
Please enter the symbol you want to search for: FB
How many quotes of FB would you like to view? 50
Invalid size passed
Please enter the symbol you want to search for: ZNGW
ZNGW not found.
Please enter the symbol you want to search for: AAPL
How many quotes of AAPL would you like to view? 5
AAPL: 119.73 441.94 442.73 430.12 417.71
Please enter the symbol you want to search for: QUIT
No Memory Leak
Press any key to continue . . .



*/