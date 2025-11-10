#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_QUESTIONS 500
#define OPTIONS 4
#define MAX_LEN 150
#define MAX_PLAYERS 10
// We now use a .txt file for questions!
#define QUESTION_FILE "questions.txt"
#define SCORE_FILE "scoreboard.dat" // Scores can stay binary

typedef struct {
    char question[MAX_LEN];
    char options[OPTIONS][MAX_LEN];
    char correct;
} QuizItem;

typedef struct {
    char name[MAX_LEN];
    int score;
} ScoreEntry;

// --- Function Prototypes ---
void shuffle(int *array, int n);
void add_question(QuizItem quiz[], int *n);
void delete_question(QuizItem quiz[], int *n);
int start_quiz(QuizItem quiz[], int n, char *player_name);
void save_questions_txt(QuizItem quiz[], int n);
void load_questions_txt(QuizItem quiz[], int *n);
void save_scores(ScoreEntry scores[], int count);
void load_scores(ScoreEntry scores[], int *count);
void strip_newline(char *str);

// --- New Helper Function ---
// fgets keeps the \n, we need to remove it.
void strip_newline(char *str) {
    str[strcspn(str, "\n")] = 0;
}

// --- Question/Quiz Logic ---

void shuffle(int *array, int n) {
    for(int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

void add_question(QuizItem quiz[], int *n) {
    if(*n >= MAX_QUESTIONS) {
        printf("Reached max number of questions (%d). Cannot add more.\n", MAX_QUESTIONS);
        return;
    }

    getchar(); // clear input buffer
    printf("Enter question: ");
    fgets(quiz[*n].question, MAX_LEN, stdin);
    strip_newline(quiz[*n].question);

    for(int i = 0; i < OPTIONS; i++) {
        printf("Enter option %c: ", 'A'+i);
        fgets(quiz[*n].options[i], MAX_LEN, stdin);
        strip_newline(quiz[*n].options[i]);
    }

    printf("Enter correct option (A/B/C/D): ");
    scanf(" %c", &quiz[*n].correct);
    if(quiz[*n].correct >= 'a' && quiz[*n].correct <= 'd')
        quiz[*n].correct -= 32;

    (*n)++;
    printf("Question added successfully!\n");
}

void delete_question(QuizItem quiz[], int *n) {
    if(*n == 0) {
        printf("No questions to delete.\n");
        return;
    }
    printf("Available questions:\n");
    for(int i = 0; i < *n; i++) {
        printf("%d. %s\n", i+1, quiz[i].question);
    }

    int del;
    printf("Enter question number to delete: ");
    scanf("%d", &del);

    if(del < 1 || del > *n) {
        printf("Invalid question number.\n");
        return;
    }

    for(int i = del - 1; i < *n - 1; i++) {
        quiz[i] = quiz[i+1];
    }
    (*n)--;
    printf("Question deleted successfully!\n");
}

int start_quiz(QuizItem quiz[], int n, char *player_name) {
    if (n == 0) {
        printf("No questions available to start quiz.\n");
        return 0;
    }

    int score = 0;
    int order[MAX_QUESTIONS];
    for (int i = 0; i < n; i++) order[i] = i;

    shuffle(order, n);

    int num_quiz_questions = (n > 10) ? 10 : n;
    char answer;

    for (int i = 0; i < num_quiz_questions; i++) {
        int idx = order[i];
        printf("\nQ%d: %s\n", i + 1, quiz[idx].question);
        for (int j = 0; j < OPTIONS; j++) {
            printf("%c) %s\n", 'A' + j, quiz[idx].options[j]);
        }
        printf("Your answer (A/B/C/D): ");
        scanf(" %c", &answer);
        if (answer >= 'a' && answer <= 'd') answer -= 32;

        if (answer == quiz[idx].correct) {
            printf("Correct!\n");
            score++;
        }
        else {
            printf("Wrong! Correct answer: %c\n", quiz[idx].correct);
        }
    }
    printf("\n%s, your final score is %d out of %d\n", player_name, score, num_quiz_questions);
    return score;
}

// --- NEW File Handling Logic for .txt ---

// NEW function to save in .txt format
void save_questions_txt(QuizItem quiz[], int n) {
    FILE *f = fopen(QUESTION_FILE, "w"); // "w" = write text
    if (f == NULL) {
        printf("Error: Could not save questions to file.\n");
        return;
    }
    
    for (int i = 0; i < n; i++) {
        fprintf(f, "%s\n", quiz[i].question);
        for (int j = 0; j < OPTIONS; j++) {
            fprintf(f, "%s\n", quiz[i].options[j]);
        }
        fprintf(f, "%c\n", quiz[i].correct);
        fprintf(f, "---\n"); // Separator
    }
    fclose(f);
}

// NEW function to load from .txt format
void load_questions_txt(QuizItem quiz[], int *n) {
    FILE *f = fopen(QUESTION_FILE, "r"); // "r" = read text
    if (f == NULL) {
        printf("No 'questions.txt' file found. Please create it.\n");
        printf("Starting with 0 questions.\n");
        *n = 0;
        return;
    }
    
    *n = 0; // Reset question count
    char line_buffer[MAX_LEN];

    // Read line by line
    while (*n < MAX_QUESTIONS && 
           fgets(quiz[*n].question, MAX_LEN, f) != NULL) 
    {
        // Read the 4 options
        for (int j = 0; j < OPTIONS; j++) {
            if (fgets(quiz[*n].options[j], MAX_LEN, f) == NULL) {
                printf("Error: File ended unexpectedly at question %d\n", *n + 1);
                return; // Stop loading
            }
        }
        
        // Read the correct answer
        if (fgets(line_buffer, MAX_LEN, f) == NULL) {
            printf("Error: File ended unexpectedly at question %d\n", *n + 1);
            return;
        }
        quiz[*n].correct = line_buffer[0]; // Get the first char ('A', 'B', 'C', or 'D')

        // Strip newlines from all read strings
        strip_newline(quiz[*n].question);
        for (int j = 0; j < OPTIONS; j++) {
            strip_newline(quiz[*n].options[j]);
        }
        
        // --- THIS IS THE FIX ---
        // We increment the count *before* looking for the separator.
        (*n)++; // Increment question count

        // Read the separator "---"
        if (fgets(line_buffer, MAX_LEN, f) == NULL) {
            // This is now OK! It just means it's the end of the file.
            break; 
        }
    }

    fclose(f);
    printf("Loaded %d questions from 'questions.txt'.\n", *n);
}


// --- Scoreboard File Handling (Unchanged) ---

void save_scores(ScoreEntry scores[], int count) {
    FILE *f = fopen(SCORE_FILE, "wb");
    if (f == NULL) {
        printf("Error: Could not save scores to file.\n");
        return;
    }
    fwrite(&count, sizeof(int), 1, f);
    fwrite(scores, sizeof(ScoreEntry), count, f);
    fclose(f);
}

void load_scores(ScoreEntry scores[], int *count) {
    FILE *f = fopen(SCORE_FILE, "rb");
    if (f == NULL) {
        printf("No score file found. Starting fresh scoreboard.\n");
        *count = 0;
        return;
    }
    fread(count, sizeof(int), 1, f);
    if (*count > MAX_PLAYERS) *count = MAX_PLAYERS;
    fread(scores, sizeof(ScoreEntry), *count, f);
    fclose(f);
    printf("Loaded %d scores from file.\n", *count);
}

// --- Main Function ---

int main() {
    srand(time(NULL));

    QuizItem quiz[MAX_QUESTIONS];
    int n = 0; // Question count

    ScoreEntry scoreboard[MAX_PLAYERS];
    int playerCount = 0;
    
    // Load all data
    load_questions_txt(quiz, &n); // Use new text loader
    load_scores(scoreboard, &playerCount);

    int choice;
    char player_name[MAX_LEN];

    do {
        printf("\nQuiz Menu:\n");
        printf("1. Start Quiz\n");
        printf("2. Add Question\n");
        printf("3. Delete Question\n");
        printf("4. Show Scoreboard\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch(choice) {
            case 1:
                if (playerCount >= MAX_PLAYERS) {
                    printf("Scoreboard full, cannot add new player.\n");
                    break;
                }
                printf("Enter your name: ");
                getchar(); // clear buffer
                fgets(player_name, MAX_LEN, stdin);
                strip_newline(player_name);

                int score = start_quiz(quiz, n, player_name);
                strcpy(scoreboard[playerCount].name, player_name);
                scoreboard[playerCount].score = score;
                playerCount++;
                save_scores(scoreboard, playerCount); 
                break;
            case 2:
                add_question(quiz, &n);
                save_questions_txt(quiz, n); // Use new text saver
                break;
            case 3:
                delete_question(quiz, &n);
                save_questions_txt(quiz, n); // Use new text saver
                break;
            case 4:
                printf("\nScoreboard:\n");
                if (playerCount == 0) {
                    printf("Scoreboard is empty.\n");
                }
                for (int i = 0; i < playerCount; i++) {
                    printf("%d. %s - %d\n", i + 1, scoreboard[i].name, scoreboard[i].score);
                }
                break;
            case 5:
                printf("Exiting...\n");
                break;
            default:
                printf("Invalid choice.\n");
        }
    } while (choice != 5);

    return 0;
}
