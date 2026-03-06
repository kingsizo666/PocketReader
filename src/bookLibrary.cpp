#include "bookLibrary.h"

// Hardcoded for now (later you’ll scan LittleFS /books)
static const BookInfo BOOKS[] = {
  { "Dune",          "/books/dune" },
  { "Hobbit",        "/books/hobbit" },
  { "Atomic Habits", "/books/atomic_habits" }
};

namespace BookLibrary {
  int count() { return (int)(sizeof(BOOKS) / sizeof(BOOKS[0])); }
  const BookInfo& get(int index) { return BOOKS[index]; }
}