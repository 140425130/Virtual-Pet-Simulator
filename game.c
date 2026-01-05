#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <cs50.h>

#define MAX_BOOKS 6           // Максимален брой книги в библиотеката
#define MAX_NAME 50           // Максимална дължина на имена
#define SAVE_FILE "owl_save.txt" // Файл за запазване на играта

typedef struct {              // Структура за книга
    char title[100];          // Заглавие на книгата
    char author[100];         // Автор на книгата
    int is_yellow;            // 1 = Жълтата книга, 0 = нормална
    int comfort;              // Колко щастие добавя при четене
} Book;

typedef struct {              // Структура за състоянието на играта
    char owl_name[MAX_NAME];     // Име на совата
    char keeper_name[MAX_NAME];  // Име на играча
    int day;                     // Текущ ден
    int actions_left;            // Оставащи действия за деня
    int health;                  // Здраве (0-100)
    int hunger;                  // Глад (0-100)
    int happiness;               // Щастие (0-100)
    int energy;                  // Енергия (0-100)
    int devotion;                // Преданост към Жълтия крал (0-100)
    Book library[MAX_BOOKS];     // Колекция от книги
    int book_count;              // Брой налични книги
    int days_since_yellow;       // Дни от последно четене на Жълтата книга
} GameState;

Book all_books[] = {          // Всички налични книги в играта
    {"Pride and Prejudice", "Jane Austen", 0, 25},
    {"Frankenstein", "Mary Shelley", 0, 15},
    {"Dracula", "Bram Stoker", 0, 10},
    {"The King in Yellow", "Robert Chambers", 1, -20},  // Специална книга
    {"The Time Machine", "H.G. Wells", 0, 20},
    {"Sherlock Holmes", "Arthur Conan Doyle", 0, 30}
};

GameState game;               // Текущо състояние на играта
int game_running = 1;         // Дали играта продължава
int game_ended = 0;           // Дали играта е завършила

void clear_screen() {         // Изчистване на конзолата
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void press_enter() {          // Чакане на натискане на Enter
    printf("\nPress Enter to continue...");
    get_string("");
}

void print_header(const char* title) { // Показване на заглавие
    clear_screen();
    printf("\n========================================\n          OWL CARETAKER\n========================================\n");
    if (title[0] != '\0') {
        printf("%s\n", title);
        printf("----------------------------------------\n");
    }
    printf("\n");
}

void draw_owl() {             // Рисуване на ASCII art на сова според щастието
    if (game.happiness > 70) {
        printf("    ,____,\n   ( ^.^ )\n   /)  (\\\n  /_/\\/_\\\n /      \\\n");
    } else if (game.happiness > 40) {
        printf("    ,____,\n   ( o.o )\n   /)  (\\\n  /_/\\/_\\\n /      \\\n");
    } else {
        printf("    ,____,\n   ( T.T )\n   /)  (\\\n  /_/\\/_\\\n /      \\\n");
    }

    if (game.devotion > 60) {
        printf(" [A whisper: Owl? What Owl?]\n");
    } else if (game.energy < 30) {
        printf(" [Looks tired]\n");
    }
}

void clamp_stat(int* stat) {  // Ограничаване на статистика между 0 и 100
    if (*stat < 0) *stat = 0;
    if (*stat > 100) *stat = 100;
}

void init_new_game() {        // Инициализация на нова игра
    srand(time(NULL));
    print_header("LONDON, 1895");
    printf("You find a wounded owl in a dark alley at night.\nYou look into it's eyes and decide to take care of it untill it gets better.\n\n");

    printf("What is your name? ");
    strncpy(game.keeper_name, get_string(""), MAX_NAME - 1);

    printf("What will you name the owl? ");
    strncpy(game.owl_name, get_string(""), MAX_NAME - 1);

    game.day = 1;             // Начални стойности
    game.actions_left = 3;
    game.health = 60;
    game.hunger = 50;
    game.happiness = 50;
    game.energy = 70;
    game.devotion = 0;
    game.book_count = 6;
    game.days_since_yellow = 0;

    for (int i = 0; i < game.book_count; i++) {  // Копиране на книгите
        game.library[i] = all_books[i];
    }

    print_header("BEGINNING");
    printf("%s settles into your study.\nYour care begins on Day 1.", game.owl_name);
    press_enter();
}

void update_stats() {         // Обновяване на статистиките в края на деня
    game.hunger += 15;        // Естествено влошаване
    game.happiness -= 8;
    game.energy -= 5;

    if (game.hunger > 80) game.health -= 15;      // Здравето се влияе от други статистики
    if (game.happiness < 20) game.health -= 10;
    if (game.energy < 20) game.health -= 5;

    if (game.devotion > 0) {                      // Намаляване на предаността
        game.devotion -= (game.days_since_yellow > 0) ? 3 : 1;
        game.days_since_yellow++;
    }

    clamp_stat(&game.health);                     // Ограничаване на всички статистики
    clamp_stat(&game.hunger);
    clamp_stat(&game.happiness);
    clamp_stat(&game.energy);
    clamp_stat(&game.devotion);
}

void show_status() {          // Показване на текущия статус на совата
    printf("Day: %d | Actions left: %d\n\nStatus of %s:\n",
           game.day, game.actions_left, game.owl_name);

    // Дефиниране на всички статистики в масив за обработка
    struct { char* name; int value; } stats[] = {
        {"Health", game.health},
        {"Hunger", game.hunger},
        {"Happiness", game.happiness},
        {"Energy", game.energy},
        {"Devotion", game.devotion}
    };

    // Обхождане и визуализация на всички статистики
    for (int s = 0; s < 5; s++) {
        printf("%-10s ", stats[s].name);
        int bars = stats[s].value / 10;

        // Визуализация с различни символи за Devotion
        if (s == 4 && stats[s].value > 50) { // Специални символи за преданост
            for (int i = 0; i < 10; i++)
                printf(i < bars ? "✦" : "░");
        } else {
            for (int i = 0; i < 10; i++)
                printf(i < bars ? "█" : "░");
        }
        printf(" %d%%\n", stats[s].value);
    }
}

void check_endings() {        // Проверка за условия за край на играта
    if (game.devotion >= 100) {               // Край 1: Пълна преданост
        print_header("THE YELLOW KING");
        draw_owl();
        printf("\n%s vanishes\n", game.owl_name);
        printf("The room dissolves...\n you find yourself in an unfamiliar place\n Strange is the night where black stars rise,\n And strange moons circle through the skies\n But stranger still is Lost Carcosa\n You seat yourself at the root of a great tree, seriously to consider what it were best to do. \n An owl on the branch of the great decayed tree hooted dismally\n A sudden wind pushed some dry leaves and twigs from the uppermost face of the stone infront of you; You see the lowrelief letters of an\n inscription and bend to read it. God in heaven! Your name in full! -- the date of Your birth! -- the date of Your death!\n Song of my soul, my voice is dead;\n Die thou, unsung, as tears unshed\n Must die unheard in Lost Carcosa\n");
        game_ended = 1;
        press_enter();
        return;
    }
if (game.happiness == 0) {                   // Край 2: Совата умира
        print_header("THE OWL GOT BORED FROM YOU");
        draw_owl();
        printf("The owl literally died from boredom\n Final devotion: %d%%\n", game.devotion);
        game_ended = 1;
        press_enter();
        return;
    }
    if (game.health == 0) {                   // Край 2: Совата умира
        print_header("THE OWL DIED FROM UNPROPER CARE");
        draw_owl();
        printf("You bury your companion in the garden.\n Final devotion: %d%%\n", game.devotion);
        game_ended = 1;
        press_enter();
        return;
    }
    if (game.hunger == 100) {                   // Край 2: Совата умира
        print_header("THE OWL DIED FROM STARVATION");
        draw_owl();
        printf("You take one final look at the starved body of the owl.\nYou bury your companion in the garden.\n Final devotion: %d%%\n", game.devotion);
        game_ended = 1;
        press_enter();
        return;
    }

    if (game.day >= 20 && game.health > 80 && game.happiness > 70) { // Край 3: Оздравяване
        print_header("THE OWL FLIES AWAY");
        draw_owl();
        printf("\nAfter 20 days, %s has fully recovered.\nWith a final hoot, she flies out the window, free.\n", game.owl_name);
        printf("Final devotion: %d%%\n", game.devotion);
        game_ended = 1;
        press_enter();
    }
}

void feed_owl() {            // Хранене на совата
    print_header("FEEDING THE OWL");
    draw_owl();

    if (game.actions_left <= 0) {
        printf("\nNo actions left for today!\n");
        press_enter();
        return;
    }

    if (game.energy < 20) {
        printf("\n%s is too tired to eat properly.\n", game.owl_name);
        press_enter();
        return;
    }

    printf("\n1. Fresh mice (Hunger -40, Happiness +15, Energy -15)\n2. Worms (Hunger -20, Happiness +5, Energy -10)\n0. Cancel\n\nChoice: ");

    int choice = get_int("");
    if (choice == 1) {
        game.hunger -= 40; game.happiness += 15; game.energy -= 15;
        printf("\n%s enjoys the fresh mice!\n", game.owl_name);
        game.actions_left--;
    } else if (choice == 2) {
        game.hunger -= 20; game.happiness += 5; game.energy -= 10;
        printf("\n%s eats the worms.\n", game.owl_name);
        game.actions_left--;
    }

    clamp_stat(&game.hunger); clamp_stat(&game.happiness); clamp_stat(&game.energy);
    press_enter();
}

void read_to_owl() {         // Четене на книга на совата
    print_header("READING TO THE OWL");
    draw_owl();

    if (game.actions_left <= 0) {
        printf("\nNo actions left for today!\n");
        press_enter();
        return;
    }

    if (game.energy < 15) {
        printf("\n%s is too tired to listen.\n", game.owl_name);
        press_enter();
        return;
    }

    printf("\nChoose a book:\n");
    for (int i = 0; i < game.book_count; i++) {
        printf("%d. %s by %s%s\n", i + 1, game.library[i].title,
               game.library[i].author, game.library[i].is_yellow ? " [FORBIDDEN]" : "");
    }
    printf("0. Cancel\n\nChoice: ");

    int choice = get_int("");
    if (choice == 0 || choice > game.book_count) return;

    Book book = game.library[choice - 1];
    printf("\nReading '%s' aloud...\n", book.title);

    if (book.is_yellow) {                      // Специален ефект за жълтата книга
        printf("\"Along the shore the cloud waves break...\"\n");
        printf("The room grows colder. %s listens\n\n", game.owl_name);
        game.devotion += 30; game.days_since_yellow = 0;
        game.happiness += book.comfort; game.energy -= 20;
        printf("Devotion: +30%%\n");
        if (game.devotion > 50) printf(" For there be divers sorts of death\n -- some wherein the body remaineth;\n and in some it vanisheth quite away with the spirit.\n");
    } else {                                   // Нормална книга
        printf("Your voice fills the room. %s listens contentedly.\n\n", game.owl_name);
        game.happiness += book.comfort; game.energy -= 10;
        printf("Happiness: +%d\n", book.comfort);
    }

    clamp_stat(&game.devotion); clamp_stat(&game.happiness); clamp_stat(&game.energy);
    game.actions_left--;
    press_enter();
}

void rest_owl() {            // Почивка за совата
    print_header("RESTING");
    draw_owl();

    if (game.actions_left <= 0) {
        printf("\nNo actions left for today!\n");
        press_enter();
        return;
    }

    printf("\n1. Short nap (Energy +30, Hunger +10, Health +20)\n2. Long sleep (Energy +60, Hunger +20, Health +10 Uses 2 actions)\n0. Cancel\n\nChoice: ");

    int choice = get_int("");
    if (choice == 1) {
        game.energy += 30; game.hunger += 10; game.health += 20;
        printf("\n%s takes a short nap and looks refreshed.\n", game.owl_name);
        game.actions_left--;
    } else if (choice == 2 && game.actions_left >= 2) {
        game.energy += 60; game.hunger += 20; game.health += 10;
        printf("\n%s sleeps deeply and wakes up full of energy.\n", game.owl_name);
        game.actions_left -= 2;
    }

    clamp_stat(&game.energy); clamp_stat(&game.hunger);
    press_enter();
}

void next_day() {            // Преминаване към следващия ден
    print_header("NEXT DAY");
    printf("\nDay %d ends. Day %d begins.\n\n", game.day, game.day + 1);

    game.day++; game.actions_left = 3;
    update_stats();

    int event = rand() % 100;                  // Случайни събития
    if (event < 30) {
        printf("%s finds something interesting! Happiness +10%%\n", game.owl_name);
        game.happiness += 10; clamp_stat(&game.happiness);
    } else if (event < 35 && game.devotion > 40) {
        printf("Strange dreams of a city... Devotion +3%%\n");
        game.devotion += 3; clamp_stat(&game.devotion);
    } else if (event < 50 && game.energy < 50) {
        printf("%s woke up well-rested. Energy +15%%\n", game.owl_name);
        game.energy += 15; clamp_stat(&game.energy);
    }

    press_enter();
}

void save_game() {           // Запазване на играта във файл
    FILE* f = fopen(SAVE_FILE, "w");
    if (f) {
        fprintf(f, "%s\n%s\n", game.keeper_name, game.owl_name);
        fprintf(f, "%d %d %d %d %d %d %d %d %d\n",
                game.day, game.actions_left, game.health, game.hunger,
                game.happiness, game.energy, game.devotion,
                game.book_count, game.days_since_yellow);
        fclose(f);
        printf("\nGame saved successfully.\n");
        press_enter();
    }
}

void load_game() {           // Зареждане на игра от файл
    FILE* f = fopen(SAVE_FILE, "r");
    if (f) {
        fgets(game.keeper_name, MAX_NAME, f);
        game.keeper_name[strcspn(game.keeper_name, "\n")] = 0;
        fgets(game.owl_name, MAX_NAME, f);
        game.owl_name[strcspn(game.owl_name, "\n")] = 0;

        fscanf(f, "%d %d %d %d %d %d %d %d %d",
               &game.day, &game.actions_left, &game.health, &game.hunger,
               &game.happiness, &game.energy, &game.devotion,
               &game.book_count, &game.days_since_yellow);

        fclose(f);
        for (int i = 0; i < game.book_count; i++) game.library[i] = all_books[i];

        print_header("GAME LOADED");
        printf("Welcome back, %s. %s awaits you.\n", game.keeper_name, game.owl_name);
        press_enter();
    } else {
        printf("No save file found.\n");
        press_enter();
        init_new_game();
    }
}

int main() {                 // Главна функция
    clear_screen();
    printf("\nOWL CARETAKER\n====================\n\n");
    printf("1. New Game\n2. Load Game\n3. Exit\n\nChoice: ");

    int choice = get_int("");
    if (choice == 1) init_new_game();
    else if (choice == 2) load_game();
    else return 0;

    while (game_running && !game_ended) {  // Главен игров цикъл
        print_header("MAIN MENU");
        draw_owl();
        show_status();

        if (game.actions_left > 0) {
            printf("\n1. Feed the owl\n2. Read to the owl\n3. Let the owl rest\n");
            printf("4. Next day\n5. Save and exit\n\nChoice: ");

            switch (get_int("")) {
                case 1: feed_owl(); break;
                case 2: read_to_owl(); break;
                case 3: rest_owl(); break;
                case 4: next_day(); break;
                case 5: save_game(); game_running = 0; break;
                default: printf("\nInvalid choice.\n"); press_enter();
            }
        } else {
            printf("\nNo actions left for today.\n1. Go to next day\n2. Save and exit\n\nChoice: ");

            choice = get_int("");
            if (choice == 1) next_day();
            else if (choice == 2) { save_game(); game_running = 0; }
        }

        check_endings();  // Проверка за край на играта
    }

    if (!game_ended) { print_header("GOODBYE"); printf("\nGame saved. Goodbye!\n"); }
    return 0;
}
