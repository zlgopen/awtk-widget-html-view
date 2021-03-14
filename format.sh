find src/html_view -name \*.c -exec clang-format -i {} \;
find src/html_view -name \*.h -exec clang-format -i {} \;
find src/html_view -name \*.cc -exec clang-format -i {} \;
find src/html_view -name \*.cpp -exec clang-format -i {} \;
find src/html_view -name \*.inc  -exec clang-format -i {} \;

