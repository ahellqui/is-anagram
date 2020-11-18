#include <iostream>
#include <array>
#include <vector>
#include <cstring>

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
    dictionary (const char* filename);

    std::vector<const char*> words;
    // Lookup table for where the different words start in the vector.
    // I'm lazy, so I'll just assume that no word exceeds 99 characters (index 0 won't be used)
    std::array<int, 100> word_lengths;

    // These are only for prototyping. In the final version, there will be a constructor
    // that takes a file name as argument and does all this
    void read_words_file ();
    void initiate_lookup_table ();
};

dictionary::dictionary (const char* filename)
{
    this->read_words_file ();
    this->initiate_lookup_table ();
}

void dictionary::read_words_file ()
{
    // in the real program, this will allocate memory and assign it to a char* (not const)
    this->words.push_back ("lo");
    this->words.push_back ("hel");

    this->words.push_back ("fjas");
    this->words.push_back ("leloh");
    this->words.push_back ("olleh");

    // sort
}

// Assumes sorted dictionary
void dictionary::initiate_lookup_table ()
{
    int last_size = strlen (this->words [0]);
    int tmp_size = 0;

    for (int i = 0; i < this->words.size (); i++)
    {
        if ((tmp_size = (strlen (this->words [i]))) > last_size)
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

bool search_anagrams_traverse (input_word input, const dictionary& dict, int dict_index, std::vector<const char*>& annagram_list)
{
    int word_length = strlen (dict.words [dict_index]);
    for (int i = 0; i < word_length; i++)
    {
        // I should probably just define a custom operator instead of doing this
        // - 'a' thing
        if (input.word_letters [dict.words [dict_index][i] - 'a'] > 0)
        {
            --(input.word_letters [dict.words [dict_index][i] - 'a']);
            --input.length;
        }
        else
        {
            return false;
        }

        if (input.length == 0)
        {
            annagram_list.push_back (dict.words [dict_index]);
            return true;
        }
        else if (i == word_length - 1 && dict_index != 0)
        {
            annagram_list.push_back (dict.words [dict_index]);
            return search_anagrams_traverse (input, dict, dict_index - 1, annagram_list);
        }
    }
    return false;
}

void search_anagrams (const input_word& input, const dictionary& dict, std::vector<std::vector<const char*>>& annagram_list)
{
    annagram_list.push_back (std::vector<const char*> ());
    for (int i = dict.word_lengths [input.length]; i >= 0; i--)
    {
        // Every iteration needs a copy of the input word so we don't change it
        input_word input_copy = input;

        if (search_anagrams_traverse (input_copy, dict, i, annagram_list.back ()) == true)
        {
            // Save the result by pushing a new element
            annagram_list.push_back (std::vector<const char*> ());
        }
        else
        {
            annagram_list.back ().clear ();
        }
    }
}

int main ()
{
    input_word word ("hello");
    dictionary dict ("hdh");

    std::vector<std::vector<const char*>> annagram_list;
    search_anagrams (word, dict, annagram_list);

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
