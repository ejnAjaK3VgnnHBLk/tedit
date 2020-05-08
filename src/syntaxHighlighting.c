#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "tedit.h"

int is_seperator(int c)
{
	return isspace(c) || c == '\0' || strchr(",.()+-/*=~%<>[];", c) != NULL;
}

void editorUpdateSyntax(erow *row)
{
	row->hl = realloc(row->hl, row->rsize);
	memset(row->hl, HL_NORMAL, row->rsize);

	if (E.syntax == NULL) return;

	char **keywords = E.syntax->keywords;

	char *scs = E.syntax->singleline_comment_start;
	char *mcs = E.syntax->multiline_comment_start;
	char *mce = E.syntax-> multiline_comment_end;

	int scs_len = scs ? strlen(scs) : 0;
	int mcs_len = mcs ? strlen(mcs) : 0;
	int mce_len = mce ? strlen(mce) : 0;

	int prev_sep = 1;
	int in_string = 0;
	int in_comment = (row->idx > 0 && E.row[row->idx - 1].hl_open_comment);

	int i = 0;
	while (i < row->rsize)
	{
		char c = row->render[i];
		unsigned char prev_hl = (i>0) ? row->hl[i - 1] : HL_NORMAL;

		if (scs_len && !in_string && !in_comment)
		{
			if (!strncmp(&row->render[i], scs, scs_len))
			{
				memset(&row->hl[i], HL_COMMENT, row->rsize - i);
				break;
			}
		}
		
		if (mcs_len && mce_len && !in_string)
		{
			if (in_comment)
			{
				row->hl[i] = HL_MLCOMMENT;
				if (!strncmp(&row->render[i], mce, mce_len))
				{
					memset(&row->hl[i], HL_MLCOMMENT, mce_len);
					i += mce_len;
					in_comment = 0;
					prev_sep = 1;
					continue;
				} else {
					i++;
					continue;
				}
			} else if (!strncmp(&row->render[i], mcs, mcs_len)) {
				memset(&row->hl[i], HL_MLCOMMENT, mcs_len);
				i += mcs_len;
				in_comment = 1;
				continue;
			}
		} else if (!strncmp(&row->render[i], mcs, mcs_len)) {
			memset(&row->hl[i], HL_MLCOMMENT, mcs_len);
			i += mcs_len;
			in_comment = 1;
			continue;
		}

		if(E.syntax->flags & HL_HIGHLIGHT_STRINGS)
		{
			if (in_string)
			{
				row->hl[i] = HL_STRING;
				if (c == '\\' && i + 1 <row->rsize)
				{
					row->hl[i + 1] = HL_STRING;
					i += 2;
					continue;
				}
				if (c == in_string) in_string = 0;
				i++;
				prev_sep = 1;
				continue;
			} else {
				if (c == '"' || c == '\'')
				{
					in_string = c;
					row->hl[i] = HL_STRING;
					i++;
					continue;
				}
			}
		}

		if (E.syntax->flags & HL_HIGHLIGHT_NUMERS)
		{
			if ((isdigit(c) && (prev_sep || prev_hl == HL_NUMBER)) ||
				(c == '.' && prev_hl == HL_NUMBER))
			{
				row->hl[i] = HL_NUMBER;
				i++;
				prev_sep = 0;
				continue;
			}
		}
		
		if (prev_sep)
		{
			int j;
			for (j = 0; keywords[j]; j++)
			{
				int klen = strlen(keywords[j]);
				int kw2 = keywords[j][klen - 1] == '|';
				if (kw2) klen--;

				if (!strncmp(&row->render[i], keywords[j], klen) &&
					is_seperator(row->render[i + klen]) )
					{
						memset(&row->hl[i], kw2 ? HL_KEYWORD2 : HL_KEYWORD1, klen);
						i+= klen;
						break;
					}
			}
			if (keywords[j] != NULL) 
			{
				prev_sep = 0;
				continue;
			}
		}

		prev_sep = is_seperator(c);
		i++;
	}

	int changed = (row->hl_open_comment != in_comment);
	row->hl_open_comment = in_comment;
	if (changed && row->idx + 1 < E.numrows)
		editorUpdateSyntax(&E.row[row->idx + 1]);
}

int editorSyntaxToColor(int hl)
{
	switch (hl)
	{
		case HL_COMMENT:
		case HL_MLCOMMENT: return 36; // Multi line comments are cyan
		case HL_KEYWORD1: return 33; // keyword 1 in yellow 
		case HL_KEYWORD2: return 32; // keyword 2 in green
		case HL_STRING: return 35; // Strings are magenta
		case HL_NUMBER: return 31; // Numbers are red
		case HL_MATCH: return 34; // Forground blue
		default: return 37;
	}
}
