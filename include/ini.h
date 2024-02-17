#ifndef INI_H
#define INI_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INI_PANIC(format, ...)                                                 \
    do {                                                                       \
        fprintf(stderr, "\033[1;31mERROR\033[0m: %s:%d: " format "\n",         \
                __FILE__, __LINE__, ##__VA_ARGS__);                            \
        exit(1);                                                               \
    } while (0)

#define INI_INIT_CAPACITY 8192
#define INI_DA_REALLOC(oldptr, oldsz, newsz) realloc(oldptr, newsz)
#define ini_da_append(da, item)                                                \
    do {                                                                       \
        if ((da)->count >= (da)->capacity) {                                   \
            size_t new_capacity = (da)->capacity * 2;                          \
            if (new_capacity == 0) {                                           \
                new_capacity = INI_INIT_CAPACITY;                              \
            }                                                                  \
                                                                               \
            (da)->items = INI_DA_REALLOC(                                      \
                (da)->items, (da)->capacity * sizeof((da)->items[0]),          \
                new_capacity * sizeof((da)->items[0]));                        \
            (da)->capacity = new_capacity;                                     \
        }                                                                      \
                                                                               \
        (da)->items[(da)->count++] = (item);                                   \
    } while (0)

// INI File Structure
struct ini_file;

void ini_parse(struct ini_file *ini, char *input);
char *ini_get_value(struct ini_file *ini, const char *section, const char *key);
void ini_free(struct ini_file *ini);

#endif // INI_H

#ifdef INI_IMPLEMENTATION

// String View
struct string_view {
        char *data;
        unsigned int len;
};

// TOKEN
enum token_type { ILLEGAL, END, SECTION, KEY, VALUE };

const char *token_type_str(enum token_type type) {
    switch (type) {
    case ILLEGAL:
        return "ILLEGAL";
    case END:
        return "END";
    case SECTION:
        return "SECTION";
    case KEY:
        return "KEY";
    case VALUE:
        return "VALUE";
    default:
        return "UNKNOWN";
    }
}

struct token {
        enum token_type type;
        struct string_view literal;
};

// Character Utilities

int is_letter(char ch) {
    return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z');
}

int is_digit(char ch) { return '0' <= ch && ch <= '9'; }

int is_valid_key_char(char ch) {
    return is_letter(ch) || is_digit(ch) || ch == '_';
}

int is_valid_section_char(char ch) {
    return is_letter(ch) || is_digit(ch) || ch == '_';
}

int is_space(char ch) { return ch == ' ' || ch == '\t'; }

int is_whitespace(char ch) {
    return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r';
}

// Lexer

struct lexer {
        char *input;
        unsigned int pos;
        unsigned int read_pos;
        char ch;
};

void lexer_init(struct lexer *l, char *input);
char lexer_read_char(struct lexer *l);
char lexer_peek_char(struct lexer *l);
struct token lexer_next_token(struct lexer *l);

void skip_whitespaces(struct lexer *l) {
    while (is_whitespace(l->ch)) {
        lexer_read_char(l);
    }
}

void skip_spaces(struct lexer *l) {
    while (is_space(l->ch)) {
        lexer_read_char(l);
    }
}

void skip_line(struct lexer *l) {
    while (l->ch != '\n' && l->ch != 0) {
        lexer_read_char(l);
    }
}

void lexer_init(struct lexer *l, char *input) {
    l->input = input;
    l->pos = 0;
    l->read_pos = 0;
    l->ch = 0;

    lexer_read_char(l);
}

char lexer_read_char(struct lexer *l) {
    l->ch = lexer_peek_char(l);

    l->pos = l->read_pos;
    l->read_pos += 1;

    return l->ch;
}

char lexer_peek_char(struct lexer *l) {
    if (l->read_pos >= strlen(l->input)) {
        return 0;
    }

    return l->input[l->read_pos];
}

struct token lexer_next_token(struct lexer *l) {
    skip_whitespaces(l);

    if (l->ch == 0) {
        return (struct token){.type = END};
    }

    if (l->ch == ';' || l->ch == '#') {
        skip_line(l);
        return lexer_next_token(l);
    }

    if (l->ch == '[') {
        lexer_read_char(l);
        struct string_view literal = {.data = l->input + l->pos, .len = 0};
        while (l->ch != ']' && l->ch != 0) {
            lexer_read_char(l);
            literal.len += 1;
        }
        lexer_read_char(l);

        return (struct token){.type = SECTION, .literal = literal};
    }

    if (is_letter(l->ch) || l->ch == '_') {
        struct string_view literal = {.data = l->input + l->pos, .len = 0};
        while (is_valid_key_char(l->ch)) {
            lexer_read_char(l);
            literal.len += 1;
        }

        return (struct token){.type = KEY, .literal = literal};
    }

    if (l->ch == '=') {
        lexer_read_char(l);
        skip_spaces(l);
        struct string_view literal = {.data = l->input + l->pos, .len = 0};
        unsigned int start = l->pos;
        skip_line(l);
        literal.len = l->pos - start;

        return (struct token){.type = VALUE, .literal = literal};
    }

    struct string_view literal = {.data = l->input + l->pos, .len = 1};
    return (struct token){.type = ILLEGAL, .literal = literal};
}

// Parser

struct key_value {
        struct string_view key;
        struct string_view value;
};

struct section {
        struct string_view name;
        struct key_value *items;
        unsigned int count;
        unsigned int capacity;
};

void ini_section_free(struct section *s) {
    if (s->items) free(s->items);
    s->items = NULL;
    s->count = 0;
    s->capacity = 0;
}

struct ini_file {
        struct section root;
        struct section *items;
        unsigned int count;
        unsigned int capacity;
};

void ini_free(struct ini_file *ini) {
    ini_section_free(&ini->root);
    if (ini->items) {
        for (unsigned int i = 0; i < ini->count; i++) {
            ini_section_free(&ini->items[i]);
        }
        free(ini->items);
    }
    ini->items = NULL;
    ini->count = 0;
    ini->capacity = 0;
}

struct token_array {
        struct token *items;
        unsigned int count;
        unsigned int capacity;
};

void token_array_free(struct token_array *tokens) {
    if (tokens->items) free(tokens->items);
    tokens->items = NULL;
    tokens->count = 0;
    tokens->capacity = 0;
}

struct parser {
        struct token_array *tokens;
        unsigned int pos;
        unsigned int read_pos;
        struct token *tok;
};

struct token parser_peek_token(struct parser *p) {
    if (p->read_pos >= p->tokens->count) {
        return (struct token){.type = END};
    }

    return p->tokens->items[p->read_pos];
}

void parser_read_token(struct parser *p) {
    if (p->read_pos >= p->tokens->count) {
        p->tok = &(p->tokens->items[p->tokens->count - 1]);
    } else {
        p->tok = &(p->tokens->items[p->read_pos]);
    }

    p->pos = p->read_pos;
    p->read_pos += 1;
}

void key_value_parse(struct key_value *kv, struct parser *p) {
    struct token tok = *p->tok;

    parser_read_token(p);
    struct token v = *p->tok;

    if (v.type != VALUE) {
        const char *type = token_type_str(v.type);
        INI_PANIC("Expected value, got token type %s (%.*s)", type,
                  v.literal.len, v.literal.data);
    }

    parser_read_token(p);

    kv->key = tok.literal;
    kv->value = v.literal;
}

void section_parse(struct section *s, struct parser *p) {
    struct token tok = *p->tok;

    while (tok.type != END && tok.type != SECTION) {
        switch (tok.type) {
        case KEY: {
            struct key_value kv;
            key_value_parse(&kv, p);
            ini_da_append(s, kv);
            break;
        }
        default: {
            const char *type = token_type_str(tok.type);
            INI_PANIC("Expected key, got token type %s (%.*s)", type,
                      tok.literal.len, tok.literal.data);
            break;
        }
        }

        tok = *p->tok;
    }
}

void ini_file_parse(struct ini_file *ini, struct parser *p) {
    struct token tok = *p->tok;

    while (tok.type != END) {
        switch (tok.type) {
        case KEY: {
            struct key_value kv;
            key_value_parse(&kv, p);
            ini_da_append(&ini->root, kv);
            break;
        }
        case SECTION: {
            parser_read_token(p);
            struct section s = {
                .name = tok.literal, .items = NULL, .count = 0, .capacity = 0};
            section_parse(&s, p);
            ini_da_append(ini, s);
            break;
        }
        default: {
            const char *type = token_type_str(tok.type);
            INI_PANIC("Expected key or section, got token type %s (%.*s)", type,
                      tok.literal.len, tok.literal.data);
            break;
        }
        }

        tok = *p->tok;
    }
}

// Parse INI file from input string
void ini_parse(struct ini_file *ini, char *input) {
    struct lexer l;
    lexer_init(&l, input);

    struct token_array tokens = {.items = NULL, .count = 0, .capacity = 0};
    struct token tok = lexer_next_token(&l);
    while (tok.type != END) {
        ini_da_append(&tokens, tok);
        tok = lexer_next_token(&l);
    }
    ini_da_append(&tokens, tok);

    struct parser p = {.tokens = &tokens, .pos = 0};
    parser_read_token(&p);
    ini_file_parse(ini, &p);

    token_array_free(&tokens);
}

// Get value from section and key
char *ini_get_value(struct ini_file *ini, const char *section,
                    const char *key) {
    struct section *s = NULL;

    if (strcmp(section, "root") == 0) {
        s = &ini->root;
    }

    for (unsigned int i = 0; i < ini->count; i++) {
        if (strncmp(ini->items[i].name.data, section, ini->items[i].name.len) ==
            0) {
            s = &ini->items[i];
        }
    }

    if (s == NULL) {
        return NULL;
    }

    for (unsigned int i = 0; i < s->count; i++) {
        struct string_view k = s->items[i].key;

        if (strncmp(k.data, key, k.len) == 0) {
            char *value = calloc(s->items[i].value.len + 1, sizeof(char));
            strncpy(value, s->items[i].value.data, s->items[i].value.len);
            return value;
        }
    }

    return NULL;
}

#endif // INI_IMPLEMENTATION
