/*
CS570 Operating Systems
Professor John Carroll
Huy Nguyen
Description: 
    The getword function read every single character from the stdin. 
    It differentiates between special characters and normal character.
    The function will return if it hit a special characters in some cases.
*/

#include <stdio.h>
#include "getword.h"


int getword(char *w)
{
	int count = 0;
	char iochar;
	int flag = 0; // identify a pair of single quote

	while (1)
	{
		iochar = getchar();

		//check if the array is full or not

		if (count >= (STORAGE -1))
		{
			ungetc(iochar,stdin);
			w[count] = '\0';
			return count;
		}

		/* This block code will eleminate the leading tab */
		// if (iochar == '\t')
		// 	continue;

		/* EOF case */
		if (iochar == EOF)
		{
			// No more characte to read
			if (count == 0)
			{
				w[count] = '\0';
				return -1;
			}
			if (count > 0)
			{
				w[count] = '\0';
				ungetc(iochar,stdin);
				return count;
			}
		}

		/* nextC case*/
		if (iochar == '\\')
		{
			//identify next character is meta-char,
			//or normal char or a single quote
			char nextC = getchar();

			//nextC is a meta-character
			//space and tab after nextC is a normal char
			//only the meta-character is input
			if ((nextC == '\'' || nextC == '!' || nextC  == '>' || 
				nextC == '&' || nextC == ' ' || nextC == '\\' || nextC == '\t') 
				&& flag == 0)
			{
				w[count++] = nextC;
				continue;
			}
			//in a pair of single quote
			//slash and meta char put in the array
			else if ( (nextC == '!' || nextC  == '>' || 
				nextC == '&' || nextC == ' ' || nextC == '\\' || nextC == ';' || nextC == '\t') 
				&& flag == 1)
			{
				w[count++] = iochar;
				w[count++] = nextC;
				continue;
			}
			//the single quote in a pair of single quote
			//treat as a normal character
			else if (nextC == '\'' && flag == 1)
			{
				w[count++] = nextC;
				continue;
			}
			// else if (nextC == '\\' || nextC )
			// {
			// 	w[count++] = nextC;
			// 	continue;
			// }
			//if is a single backnextC
			//terminate getword() call 
			//return the string

			//return the string if it encounters new line
			//put nextC back to stdin for next call
			//because we need to print out to determine the new line exist
			else if (nextC == '\n' || nextC == ';')
			{
				w[count] = '\0';
				ungetc(nextC,stdin);
				return count;
			}
			else
			{
				//ungetc(nextC,stdin); 
				//w[count] = '\0';
				w[count++] = nextC;
				//return count;
				continue;
			}
		}// end if blacknextC

		/* This block code will eleminate the leading tab */
		/* Space case and tab case*/
		if (iochar == ' ' || iochar == '\t')
		{
			if (flag == 1) //in single quote
			{	
				w[count++] = iochar;
				continue;
			}
			else if (count == 0)//ignore if it is leading char or space char
				continue;
			else
			{	//blank is delimeter
				w[count] = '\0';
				return count;
			}
		}// end if space

		/* Single quote case*/
		if (iochar == '\'')
		{
			//if the single quote is leading character
			if (flag == 0)
			{
				flag = 1;
				continue;
			}
			//if the single quote is the second single quote
			//ungetc() the single quote to start a pair of single quote if it happend
			if (flag == 1)
			{
				char c = getchar();
				//if it's metachar
				//return string before it
				//ungetc() the metachar back to stdin
				//for next call to deterine EOF or newline or metachar
				if (c == '!' || c  == '>' || 
					c == '&' || c == ' ' || c == '\\' || c == '\t' || c == '\n' || c == '|')
				{
					w[count] = '\0';
					ungetc(c,stdin);
					return count;
				}
				//back to normal single quote
				//ignore that single quote *****
				else
				{
					w[count++] = c;
					continue;
				}
				//Save point
				// w[count] = '\0';
				// ungetc(iochar,stdin);
				// flag = 0;
				// return count;
			}
			//if it is first single quote and a delimeter
			//ungetc() the single quote to start a pair of single quote if it happend
			// if (flag == 0 && count > 0)
			// {
			// 	w[count] = '\0';
			// 	ungetc(iochar,stdin);
			// 	return count;
			// }
		}//end if single quote

		/* Meta character case */
		if (iochar == '>' || iochar == '<' || iochar == '!' 
			|| iochar == ';' || iochar == '\n' || iochar == '&' || iochar == '|')
		{
			//Special character ">!"
			if (iochar == '>')
			{
				char c = getchar();
				//return the string if is a delimeter
				//put two characters back to stdin for next getword() call
				if ( c == '!' && count > 0 && flag == 0)//outside single quote
				{
					w[count] = '\0';
					ungetc(c,stdin);	
					ungetc(iochar,stdin);
					return count;
				}
				//put the delimeter to the array as a delimeter
				if (c == '!' && count == 0 && flag == 0)//outside single quote
				{
					w[count++] = iochar; //iochar = ">"
					w[count++] = c; //c = "!"
					w[count] = '\0';
					return count;
				}
				//put c and iochar back to stdin for next call
				//make sure in order, ">" need to be read first
				if ( c != '!' && count > 0 && flag == 0)//outside single quote
				{
					ungetc(c,stdin);
					ungetc(iochar,stdin);
					w[count] = '\0';
					return count;
				}
				//put the ">" in to the array
				//make sure put the non-"!" back to stdin for next call
				if ( c != '!' && count == 0 && flag == 0)//outside single quote
				{
					ungetc(c,stdin);
					w[count++] = iochar;
					w[count] = '\0';
					return count;
				}

				/****Inside single quote ***/
				if ( c == '!' && flag == 1) // read both '>' and '!'
				{
					w[count++] = iochar;
					w[count++] = c;
					continue;
				}
				//put the char that is NOT '!' back to stdin for next call
				if (c != '!' && flag == 1)// only read '>' 
				{
					ungetc(c,stdin);
					w[count++] = iochar;
					continue;
				}

			}

			//If the character is a new line "\n" or ";"
			if (iochar == '\n' || iochar == ';')
			{
				if (count == 0) // return an empty string
				{
					w[count] = '\0';
					return 0;
				}
				if (count > 0) // delimeter, return a string 
				{
					w[count] = '\0';
					ungetc(iochar,stdin);
					return count;
				}
			}

			//The metacharacters is '<' || '|' || '&' || '\'' 
			if (iochar == '<' || iochar == '!' || iochar == '&' || iochar == '\'' || iochar == '|')
			{
				//return the character as delimeter
				//outside the single quote
				if (count == 0 && flag == 0)
				{
					w[count++] = iochar;
					w[count] = '\0';
					return count;
				}
				//delimeter, return a string
				//put the meta-character back to stdin for next call to get the meta-character
				if (count > 0 && flag == 0) //single quote is OFF
				{
					w[count] = '\0';
					ungetc(iochar,stdin);
					return count;
				}
				//if the metacharacter inside a pair of single quote
				if ( (count == 0 && flag ==1) || (count > 0 && flag ==0) )
				{
					w[count++] = iochar;
					continue;
				}
			}
			

		}//end if meta case
		w[count++] = iochar;

	}//end while
}// getword