CC = gcc
CFLAGS = -Wall -Werror -Wextra -std=c11
CFLAGS_GTK = `pkg-config --cflags gtk4`
GCOVFLAGS = -fprofile-arcs -ftest-coverage
LDFLAGS = `pkg-config --cflags --libs check` -lm
LDFLAGS_GTK = -std=c11 -g `pkg-config --libs gtk4` -lm

SRC_DIR = .
OBJ_DIR = obj
BUILD_DIR = build
GCOV_HTML_DIR = report
OBJ_TEST_DIR = obj/test

NAME = smartcalc
CHECK_NAME = $(NAME).check
TEST_NAME = test_$(NAME)
EXECUTABLE = $(NAME)
COVERAGE_INFO = coverage.info
DIST_NAME = SmartCalc_v1.0
SRC = $(NAME).c $(SRC_MODEL)
SRC_MODEL = $(NAME)_model.c
OBJ =  $(addprefix $(OBJ_DIR)/, $(SRC:.c=.o))
OBJ_TEST = $(addprefix $(OBJ_TEST_DIR)/, $(SRC_MODEL:.c=.o))

all: clean uninstall start

clean:
	@echo "Cleaning up..."
	rm -rf *.o *.gcov *.gcda *.gcno $(GCOV_HTML_DIR) $(TEST_NAME) $(OBJ_DIR)

uninstall:
	@echo "Uninstalling..."
	rm -rf $(BUILD_DIR)

install:$(SRC)
	@echo "Installing..."
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS_GTK) $^ -o $(BUILD_DIR)/$(NAME) $(LDFLAGS_GTK)
	@cp smartcalc_view.ui $(BUILD_DIR)

start: install
	@echo "Running..."
	$(BUILD_DIR)/$(NAME)

dvi:
	open smartcalc.html

dist:
	@mkdir -p $(DIST_NAME)
	cp *.[ch] *.ui *.check *.html Makefile $(DIST_NAME)
	tar -czvf $(DIST_NAME).tar.gz $(DIST_NAME)
	rm -rf $(DIST_NAME)

test: clean $(TEST_NAME)
	@echo "Running tests..."
	./$(TEST_NAME)

valgrind_test: $(TEST_NAME)
	CK_FORK=no valgrind --leak-check=full ./$<

leaks_test: $(TEST_NAME)
	CK_FORK=no leaks -quiet --atExit -- ./$<

format_test:
	@echo "Checking styles..."
	clang-format -n -style=Google *.[ch]

gcov_report: test
	@echo "Generating HTML coverage report..."
	gcov $(SRC_MODEL)
	@mkdir -p $(GCOV_HTML_DIR)
	geninfo $(OBJ_TEST_DIR) --output-file $(GCOV_HTML_DIR)/$(COVERAGE_INFO)
	genhtml $(GCOV_HTML_DIR)/$(COVERAGE_INFO) --output-directory $(GCOV_HTML_DIR)
	open $(GCOV_HTML_DIR)/index.html

$(OBJ_TEST_DIR)/%.o: $(SRC_MODEL)
	@mkdir -p $(OBJ_TEST_DIR)
	$(CC) $(CFLAGS) $(GCOVFLAGS) -c -o $@ $^

$(TEST_NAME): $(OBJ_TEST)
	checkmk $(CHECK_NAME) | $(CC) $(GCOVFLAGS) -o $@ $^ -xc - $(LDFLAGS)


.PHONY: all clean uninstall install start dvi dist test valgrind_test leaks_test format_test gcov_report