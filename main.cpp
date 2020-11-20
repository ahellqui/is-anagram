#include <iostream>
#include <array>
#include <vector>
#include <cstring>
#include <cstdio>
#include <algorithm>

/* So I get stack overflow. I think there are two things I can do about that:
 *  - Initially remove all words from the dictionary that don't contain any letters in input
 *  - Actually use the word_lengths thing to jump less
 *
 *  I also don't know what to do about apostrophes
*/
struct input_word
{
    input_word (const char* word);
    input_word (const input_word& word);

    int length;
    int word_letters [26];
};

input_word::input_word (const char* word)
{
    this->length = strlen (word);

    // magic numbers are ugly, I know
    for (int i = 0; i < 26; i++)
        this->word_letters [i] = 0;

    for (int i = 0; i <= this->length; i++)
        word_letters [word [i] - 'a'] += 1;
}

input_word::input_word (const input_word& word)
    : length (word.length)
{
    for (int i = 0; i < 26; i++)
        this->word_letters [i] = word.word_letters [i];
}

struct dictionary
{
    dictionary () = default;
    dictionary (const char* filename);

    // Yes these are never freed. Destructors might arrive eventually when I feel like it
    std::vector<char*> words;

    // Lookup table for where the different words start in the vector.
    // I'm lazy, so I'll just assume that no word exceeds 99 characters (index 0 won't be used)
    std::array<int, 100> word_lengths {};

    void initiate_lookup_table ();

private:
    void read_dictionary_file (const char* filename);
};

dictionary::dictionary (const char* filename)
{
    this->read_dictionary_file (filename);
    this->initiate_lookup_table ();
}

void dictionary::read_dictionary_file (const char* filename)
{
    FILE* dict_file = fopen (filename, "rb");
    if (dict_file == NULL)
    {
        std::cout << "Could not open dictionary file\n";
        exit (1);
    }
    int begin_word = 0;

    int result;
    while ((result = getc (dict_file)))
    {
        if (result == EOF)
        {
            // std::cout << "IN IF\n";
            break;
        }
        else if (result == '\n')
        {
            // std::cout << "BEGIN ELSE\n";
            int word_length = ftell (dict_file) - begin_word;
            // std::cout << "WORD LENGTH: " << word_length << "\n";
            char* word = new char [word_length];
            // fgets puts a null terminator where the newline character would be
            fseek (dict_file, -word_length, SEEK_CUR);

            char tmp = getc (dict_file);
            // std::cout << "GETC: " << tmp << "\n";

            ungetc (tmp, dict_file);

            fgets (word, word_length, dict_file);
            // std::cout << "Word: " << word<< "\n";
            this->words.push_back (word);

            begin_word += word_length;
            // Get rid of the newline at the end
            getc (dict_file);
            // std::cout << "END ELSE\n";
        }
    }
    fclose (dict_file);

    std::sort (this->words.begin (), this->words.end (), [](char* a, char* b) {
            return strlen (a) > strlen (b);
            });

    /*
    for (char* c : this->words)
    {
        std::cout << c << "\n";
    }
    */
}

// Assumes sorted dictionary
void dictionary::initiate_lookup_table ()
{
    int last_size = strlen (this->words [0]);
    int tmp_size = 0;

    for (int i = 0; i < this->words.size (); i++)
    {
        if ((tmp_size = (strlen (this->words [i]))) != last_size)
        {
            // Use the indicies before the switch instead of after
            this->word_lengths [last_size] = i - 1;
            last_size = tmp_size;
        }
        else if (i == this->words.size () - 1)
        {
            this->word_lengths [last_size] = i;
        }
    }
}

// Returns a new dictionary with only letters contained in the input_word.
// The pointers are shared with the original dictionary though, so it should only
// be read from
const dictionary* filter_dictionary (const dictionary& from, const input_word& filter_word)
{
    dictionary* new_dict = new dictionary ();

    for (int i = 0; i < from.words.size (); i++)
    {
        int word_length = strlen (from.words [i]);
        for (int j = 0; j < word_length; j++)
        {
            if (filter_word.word_letters [from.words [i][j] - 'a'] == 0)
            {
                break;
            }
            else if (j == word_length - 1)
            {
                new_dict->words.push_back (from.words [i]);
            }
        }
    }

    new_dict->initiate_lookup_table ();

    return new_dict;
}

void search_anagrams_traverse (input_word input, const dictionary& dict, int dict_index, std::vector<std::vector<const char*>>& annagram_list, int* recursions)
{
    int word_length = strlen (dict.words [dict_index]);

    // std::cout << "Word length: " << word_length << "\n";
    // std::cout << "DICT WORD: " << dict.words [dict_index] << "\n";
    // std::cout << "Input length: " << input.length << "\n";

    /*
    for (int i = 0; i < 26; i++)
    {
        std::cout << char(i+97) << ": " << input.word_letters [i] << "\n";
    }
    */

    for (int i = 0; i < word_length; i++)
    {
        // I should probably just define a custom operator instead of doing this
        // - 'a' thing
        if (input.word_letters [dict.words [dict_index][i] - 'a'] > 0)
        {
            // std::cout << "Decrementing: " << dict.words [dict_index][i] << "\n";
            --(input.word_letters [dict.words [dict_index][i] - 'a']);
            --input.length;

            // std::cout << "New input length: " << input.length << "\n";
        }
        else
        {
            // std::cout << "In return false\n";
            return;
        }

        if (input.length == 0 && i == word_length - 1)
        {
            printf ("Recursions: %d\n", *recursions);
            annagram_list.push_back (annagram_list.back ());
            annagram_list [annagram_list.size () - 2].push_back (dict.words [dict_index]);
            return;
        }
        else if (i == word_length - 1)
        {
            annagram_list.back ().push_back (dict.words [dict_index]);
            for (int j = 1; dict_index + j < dict.words.size (); j++)
            {
                input_word input_copy = input;

                if (strlen (dict.words [dict_index + j]) > input.length)
                {
                    j = dict.word_lengths [input.length];
                    j -= dict_index;
                }
                // std::cout << "copy_length: " << input_copy.length << "\n";
                *recursions += 1;
                search_anagrams_traverse (input_copy, dict, dict_index + j, annagram_list, recursions);
            }
            annagram_list.back ().pop_back ();
        }
    }
    return;
}

void search_anagrams (const input_word& input, const dictionary& dict, std::vector<std::vector<const char*>>& annagram_list)
{
    annagram_list.push_back (std::vector<const char*> ());
    for (int i = dict.word_lengths [input.length]; i < dict.words.size (); i++)
    {
        int tmp = 0;
        // Every iteration needs a copy of the input word so we don't change it
        input_word input_copy = input;
        search_anagrams_traverse (input_copy, dict, i, annagram_list, &tmp);
    }
}

int main (int argc, char** argv)
{
    if (argc != 3)
    {
        std::cout << "Please provide the word to check and the dictionary file as arguments\n";
        exit (1);
    }

    input_word word (argv [1]);
    dictionary dict (argv [2]);

    const dictionary* filtered_dict = filter_dictionary (dict, word);

    std::vector<std::vector<const char*>> annagram_list;

    // for (int i = 0; i < filtered_dict->words.size (); i++)
        // std::cout << "i = " << i << ": "<< filtered_dict->words [i] << "\n";
    search_anagrams (word, *filtered_dict, annagram_list);

    delete filtered_dict;

    std::cout << "Annagrams:\n";
    // There will always be one extra element that gets created to store the next
    // result (which doesn't exist)
    for (int i = 0; i < annagram_list.size () - 1; i++)
    {
        std::cout << "{";
        for (int j = 0; j < annagram_list [i].size (); j++)
        {
            if (j != annagram_list [i].size () - 1)
                std::cout << annagram_list [i][j] << ", ";
            else
                std::cout << annagram_list [i][j];
        }
        std::cout << "}" << "\n";
    }

    return 0;
}
