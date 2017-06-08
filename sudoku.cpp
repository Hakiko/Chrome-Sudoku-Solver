#include <bits/stdc++.h>

struct removed_option {
  size_t row, column;
  size_t option;
};

struct move {
  size_t row, column;
  size_t value;
  bool sure;
  std::vector<removed_option> removed_options;
};

struct board_tile {
  size_t value;
  bool options[9];
  size_t opt_count;
  size_t obvious_index;
  size_t bad_index;
  size_t unset_index;
};

class Board {
 public:
  Board(const std::string &description) {
    m_totalGuesses = 0;
    m_totalMoves = 0;
    for (size_t i = 0; i < 9; ++i) {
      for (size_t j = 0; j < 9; ++j) {
        auto &tile = m_tiles[i][j];
        tile.value = 0;
        tile.opt_count = 9;
        tile.bad_index = -1;
        tile.obvious_index = -1;
        tile.unset_index = m_unsetTiles.size();
        m_unsetTiles.push_back({i, j});
        for (size_t k = 0; k < 9; ++k) {
          m_tiles[i][j].options[k] = true;
        }
      }
    }
    size_t pos = 0;
    for (size_t i = 0; i < 9; ++i) {
      for (size_t j = 0; j < 9; ++j) {
        if (description[pos] != '0') {
          move m;
          m.row = i;
          m.column = j;
          m.sure = false;
          m.value = description[pos] - '0' - 1;
          if (m_tiles[m.row][m.column].value != 0) {
            assert(m_tiles[m.row][m.column].value == m.value + 1);
          } else {
            build_removed_options(m);
            play_move(m);
          }
        }
        ++pos;
      }
    }
    assert(m_badTiles.empty());
    while (!m_unsetTiles.empty()) {
      auto tgt = m_unsetTiles.back();
      move m;
      assert(m_tiles[tgt.first][tgt.second].opt_count > 0);
      m.sure = m_tiles[tgt.first][tgt.second].opt_count == 1;
      m.row = tgt.first;
      m.column = tgt.second;
      m.value = -1;
      for (size_t i = 0; i < 9; ++i) {
        if (m_tiles[tgt.first][tgt.second].options[i]) {
          m.value = i;
          break;
        }
      }
      assert(m.value != size_t(-1));
      build_removed_options(m);
      m_totalGuesses += 1;
      play_move(m);
      while (!m_badTiles.empty()) {
        int count = 1;
        while (m_moves.back().sure) {
          reverse_top();
          count += 1;
        }
        move guess = m_moves.back();
        reverse_top();
        size_t skipped = 0;
        auto &tgt_tile = m_tiles[guess.row][guess.column];
        for (size_t i = 0; i <= guess.value; ++i) {
          if (tgt_tile.options[i]) {
            skipped += 1;
          }
        }
        size_t last = guess.value;
        guess.value = -1;
        for (size_t i = last + 1; i < 9; ++i) {
          if (tgt_tile.options[i]) {
            guess.value = i;
            break;
          }
        }
        assert(guess.value != size_t(-1));
        guess.sure = tgt_tile.opt_count == skipped + 1;
        guess.removed_options.clear();
        build_removed_options(guess);
        m_totalGuesses += 1;
        play_move(guess);
      }
    }
    assert(verify_board());
  }

  std::string description() const {
    std::string res;
    res.reserve(81);
    for (size_t i = 0; i < 9; ++i) {
      for (size_t j = 0; j < 9; ++j) {
        res += m_tiles[i][j].value + '0';
      }
    }
    return res;
  }

  size_t guesses() const { return m_totalGuesses; }

  size_t moves() const { return m_totalMoves; }

 private:
  void play_move(const move &m) {
    auto &move_tile = m_tiles[m.row][m.column];
    assert(move_tile.options[m.value]);
    assert(move_tile.opt_count > 0);
    assert(move_tile.bad_index == size_t(-1));
    assert(move_tile.unset_index != size_t(-1));
    for (const auto &opt : m.removed_options) {
      auto &tile = m_tiles[opt.row][opt.column];
      assert(tile.unset_index != size_t(-1));
      assert(tile.options[opt.option]);
      tile.options[opt.option] = false;
      tile.opt_count -= 1;
      if (opt.row != m.row || opt.column != m.column) {
        if (tile.opt_count == 1) {
          assert(tile.obvious_index == size_t(-1));
          tile.obvious_index = m_obviousTiles.size();
          m_obviousTiles.push_back({opt.row, opt.column});
        } else if (tile.opt_count == 0) {
          assert(tile.obvious_index != size_t(-1));
          std::swap(m_obviousTiles.back(), m_obviousTiles[tile.obvious_index]);
          m_obviousTiles.pop_back();
          if (!m_obviousTiles.empty()) {
            auto pos = m_obviousTiles[tile.obvious_index];
            m_tiles[pos.first][pos.second].obvious_index = tile.obvious_index;
          }
          tile.obvious_index = -1;
          assert(tile.bad_index == size_t(-1));
          tile.bad_index = m_badTiles.size();
          m_badTiles.push_back({opt.row, opt.column});
        }
      }
    }
    assert(move_tile.opt_count == 0);
    move_tile.value = m.value + 1;
    std::swap(m_unsetTiles.back(), m_unsetTiles[move_tile.unset_index]);
    m_unsetTiles.pop_back();
    if (!m_unsetTiles.empty()) {
      auto pos = m_unsetTiles[move_tile.unset_index];
      m_tiles[pos.first][pos.second].unset_index = move_tile.unset_index;
    }
    move_tile.unset_index = -1;
    if (move_tile.obvious_index != size_t(-1)) {
      std::swap(m_obviousTiles.back(), m_obviousTiles[move_tile.obvious_index]);
      m_obviousTiles.pop_back();
      if (!m_obviousTiles.empty()) {
        auto pos = m_obviousTiles[move_tile.obvious_index];
        m_tiles[pos.first][pos.second].obvious_index = move_tile.obvious_index;
      }
      move_tile.obvious_index = -1;
    }
    m_moves.push_back(m);
    m_totalMoves += 1;
    make_obvious();
  }

  void reverse_top() {
    move m = m_moves.back();
    m_moves.pop_back();
    auto &tile = m_tiles[m.row][m.column];
    assert(tile.value == m.value + 1);
    tile.value = 0;
    assert(tile.unset_index == size_t(-1));
    tile.unset_index = m_unsetTiles.size();
    m_unsetTiles.push_back({m.row, m.column});
    for (const auto &opt : m.removed_options) {
      auto &other = m_tiles[opt.row][opt.column];
      other.opt_count += 1;
      assert(other.options[opt.option] == false);
      other.options[opt.option] = true;
      if (other.opt_count == 1) {
        if (other.bad_index != size_t(-1)) {
          std::swap(m_badTiles.back(), m_badTiles[other.bad_index]);
          m_badTiles.pop_back();
          if (!m_badTiles.empty()) {
            auto pos = m_badTiles[other.bad_index];
            m_tiles[pos.first][pos.second].bad_index = other.bad_index;
          }
          other.bad_index = -1;
        }
        assert(other.obvious_index == size_t(-1));
        other.obvious_index = m_obviousTiles.size();
        m_obviousTiles.push_back({opt.row, opt.column});
      } else if (other.opt_count == 2) {
        assert(other.obvious_index != size_t(-1));
        std::swap(m_obviousTiles.back(), m_obviousTiles[other.obvious_index]);
        m_obviousTiles.pop_back();
        if (!m_obviousTiles.empty()) {
          auto pos = m_obviousTiles[other.obvious_index];
          m_tiles[pos.first][pos.second].obvious_index = other.obvious_index;
        }
        other.obvious_index = -1;
      }
    }
  }

  void build_removed_options(move &m) {
    for (size_t i = 0; i < 9; ++i) {
      if (i != m.row) {
        if (m_tiles[i][m.column].options[m.value]) {
          removed_option opt;
          opt.column = m.column;
          opt.row = i;
          opt.option = m.value;
          m.removed_options.push_back(opt);
        }
      }
      if (i != m.column) {
        if (m_tiles[m.row][i].options[m.value]) {
          removed_option opt;
          opt.column = i;
          opt.row = m.row;
          opt.option = m.value;
          m.removed_options.push_back(opt);
        }
      }
      size_t sq_row = (i / 3) + 3 * (m.row / 3);
      size_t sq_col = (i % 3) + 3 * (m.column / 3);
      if (sq_row != m.row && sq_col != m.column) {
        if (m_tiles[sq_row][sq_col].options[m.value]) {
          removed_option opt;
          opt.row = sq_row;
          opt.column = sq_col;
          opt.option = m.value;
          m.removed_options.push_back(opt);
        }
      }
      if (m_tiles[m.row][m.column].options[i]) {
        removed_option opt;
        opt.row = m.row;
        opt.column = m.column;
        opt.option = i;
        m.removed_options.push_back(opt);
      }
    }
  }

  void make_obvious() {
    while (!m_obviousTiles.empty()) {
      auto pos = m_obviousTiles.back();
      auto &tile = m_tiles[pos.first][pos.second];
      assert(tile.bad_index == size_t(-1));
      assert(tile.obvious_index != size_t(-1));
      assert(tile.value == 0);
      move m;
      m.row = pos.first;
      m.column = pos.second;
      m.sure = true;
      m.value = -1;
      assert(tile.opt_count == 1);
      for (size_t i = 0; i < 9; ++i) {
        if (m_tiles[pos.first][pos.second].options[i]) {
          m.value = i;
          break;
        }
      }
      assert(m.value != size_t(-1));
      build_removed_options(m);
      play_move(m);
    }
  }

  bool verify_board() const {
    for (size_t i = 0; i < 9; ++i) {
      for (size_t j = 0; j < 9; ++j) {
        auto &tile = m_tiles[i][j];
        assert(tile.bad_index == size_t(-1));
        assert(tile.unset_index == size_t(-1));
        assert(tile.opt_count == 0);
        for (size_t k = 0; k < 9; ++k) {
          if (k != i) {
            if (m_tiles[k][j].value == tile.value) {
              return false;
            }
          }
          if (k != j) {
            if (m_tiles[i][k].value == tile.value) {
              return false;
            }
          }
          size_t sq_row = (k / 3) + 3 * (i / 3);
          size_t sq_col = (k % 3) + 3 * (j / 3);
          if (sq_row != i && sq_col != j) {
            if (m_tiles[sq_row][sq_col].value == tile.value) {
              return false;
            }
          }
        }
      }
    }
    return true;
  }

 private:
  board_tile m_tiles[9][9];
  std::vector<move> m_moves;
  std::vector<std::pair<size_t, size_t>> m_badTiles;
  std::vector<std::pair<size_t, size_t>> m_obviousTiles;
  std::vector<std::pair<size_t, size_t>> m_unsetTiles;
  size_t m_totalGuesses;
  size_t m_totalMoves;
};

void dump_formatted(const std::string &description) {
  size_t pos = 0;
  for (size_t i = 0; i < 9; ++i) {
    for (size_t j = 0; j < 9; ++j) {
      printf("%c", description[pos]);
      ++pos;
      if (j % 3 == 2) {
        printf(" ");
      }
    }
    if (i % 3 == 2 && i != 8) {
      printf("\n");
    }
    printf("\n");
  }
}

//#define CHROME

int main() {
#ifdef CHROME
  char data_len_bytes[4];
  std::cin.read(data_len_bytes, 4);
  uint32_t data_len = *(uint32_t *)data_len_bytes;
  char data[256];
  std::cin.read(data, data_len);
  data[data_len] = '\0';
  std::cmatch m;
  size_t start = 0;
  while (data[start] != '\"') {
    ++start;
  }
  ++start;
  while (std::string(data + start, 5) != "board") {
    while (data[start] != '\"') {
      ++start;
    }
    ++start;
    while (data[start] != '\"') {
      ++start;
    }
    ++start;
  }
  start += 6;
  while (data[start] != ':') {
    ++start;
  }
  ++start;
  while (data[start] != '\"') {
    ++start;
  }
  ++start;
  std::string description(data + start, 81);
#else
  std::string description;
  std::cin >> description;
#endif
  Board b(description);
  std::string solved = b.description();
#ifdef CHROME
  std::string response("{\"board\":\"" + solved + "\"}");
  uint32_t response_len = response.length();
  std::cout.write(static_cast<char *>(static_cast<void *>(&response_len)), 4);
  std::cout.write(response.c_str(), response.length());
#else
  printf("%s\n", solved.c_str());
  printf("%lu %lu\n", b.guesses(), b.moves());
  dump_formatted(solved);
#endif
}
