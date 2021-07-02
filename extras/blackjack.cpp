/**
 * @file blackjack.cpp
 * @author atctwo
 * @brief a program to test a blackjack thing
 * @version 0.1
 * @date 2021-07-02
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <vector>
#include <iostream>

// std::vector<std::string> card_list = {
//     "2♥", "3♥", "4♥", "5♥", "6♥", "7♥", "8♥", "9♥", "10♥", "J♥", "Q♥", "K♥", "A♥",  // hearts
//     "2♦", "3♦", "4♦", "5♦", "6♦", "7♦", "8♦", "9♦", "10♦", "J♦", "Q♦", "K♦", "A♦",  // diamonds
//     "2♣", "3♣", "4♣", "5♣", "6♣", "7♣", "8♣", "9♣", "10♣", "J♣", "Q♣", "K♣", "A♣",  // clubs
//     "2♠", "3♠", "4♠", "5♠", "6♠", "7♠", "8♠", "9♠", "10♠", "J♠", "Q♠", "K♠", "A♠"   // spades
// };
std::vector<std::string> card_list = {
    "🂲", "🂳", "🂴", "🂵", "🂶", "🂷", "🂸", "🂹", "🂺", "🂻", "🂽", "🂾", "🂱",  // hearts
    "🃂", "🃃", "🃄", "🃅", "🃆", "🃇", "🃈", "🃉", "🃊", "🃋", "🃍", "🃎", "🃁",  // diamonds
    "🃒", "🃓", "🃔", "🃕", "🃖", "🃗", "🃘", "🃙", "🃚", "🃛", "🃝", "🃞", "🃑",  // clubs
    "🂢", "🂣", "🂤", "🂥", "🂦", "🂧", "🂨", "🂩", "🂪", "🂫", "🂭", "🂮", "🂡"   // spades
};
std::vector<int> cards_deck;
std::vector<int> player_deck;
std::vector<int> cpu_deck;

void reset_decks()
{
    cards_deck.clear();
    player_deck.clear();
    cpu_deck.clear();

    for (int i = 0; i < card_list.size(); i++) cards_deck.push_back(i);
}

void deal_card(std::vector<int> &deck)
{
    // pick a random card
    srand(time(NULL));
    int selected_card_id = rand() % cards_deck.size();
    
    // deal it to a player
    deck.push_back(cards_deck[selected_card_id]);

    // remove it from the main deck
    cards_deck.erase(cards_deck.begin() + selected_card_id);
}

int tally_cards(std::vector<int> deck)
{
    int tally = 0;
    for (int i = 0; i < deck.size(); i++)
    {
        int index = deck[i] % 13;
        if (index < 8) tally += index + 2; // if the card is 2 to 9
        else tally += 10;
    }
    return tally;
}

int main()
{
    reset_decks();

    deal_card(player_deck);
    deal_card(cpu_deck);

    bool player_stuck = false;
    bool cpu_stuck = false;

    while(true)
    {
        player_stuck = false;
        cpu_stuck = false;

        printf("Player's Turn\n");
        
        printf("Player Deck: ");
        for (int card_id : player_deck) printf("%s, ", card_list[card_id].c_str());
        printf("total = %d\n", tally_cards(player_deck));
        
        printf("Stick or Twist (s/t): ");
        std::string input;
        std::cin >> input;

        if (input == "s") player_stuck = true;
        if (input == "t") deal_card(player_deck);

        printf("Player Deck: ");
        for (int card_id : player_deck) printf("%s, ", card_list[card_id].c_str());
        printf("total = %d\n", tally_cards(player_deck));

        printf("\n\n");



        printf("CPU's Turn\n");
        int cpu_choice;

        // if cpu is bust, stick
        if (tally_cards(cpu_deck) > 21) cpu_choice = 0;

        // otherwise randomly decided whether to stick or twist
        else cpu_choice = rand() % 2;
        
        if (cpu_choice == 0) // cpu stuck
        {
            cpu_stuck = true; 
            printf("CPU Stuck\n");
        }
        else // cpu twisted
        {
            deal_card(cpu_deck);
            printf("CPU Twisted\n");
        }

        printf("\n\n");


        if (player_stuck && cpu_stuck)
        {
            printf("Round Over\n");

            int player_tally = tally_cards(player_deck);
            int cpu_tally = tally_cards(cpu_deck);

            // print decks
            printf("Player Deck: ");
            for (int card_id : player_deck) printf("%s, ", card_list[card_id].c_str());
            printf("total = %d\n", player_tally);

            printf("CPU Deck: ");
            for (int card_id : cpu_deck) printf("%s, ", card_list[card_id].c_str());
            printf("total = %d\n", cpu_tally);

            // print winnner
            if (player_tally > 21 && cpu_tally <= 21) printf("CPU Wins\n");                 // player is bust
            else if (player_tally <= 21 && cpu_tally > 21) printf("Player Wins\n");         // cpu is bust
            else if (player_tally > 21 && cpu_tally > 21)  printf("It's a Draw\n");         // both players are bust
            else {
                int player_difference = 21 - player_tally;
                int cpu_difference = 21 - cpu_tally;

                if (player_difference > cpu_difference) printf("CPU Wins\n");               // player is closer to 21
                else if (player_difference < cpu_difference) printf("Player Wins\n");       // cpu is closer to 21
                else if (player_difference == cpu_difference)  printf("It's a Draw\n");     // both players have the same score
            }

            printf("Play Again? (y/n): ");
            std::cin >> input;
            if (input == "y") {
                reset_decks();
                deal_card(player_deck);
                deal_card(cpu_deck);
            }
            else break;
        }
    }
}