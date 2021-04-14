<h1 style="font-size:64px;"> WIP </h1>

<h1> DBJ CPP TOP </h1>

*In IT systems in general, the programming language is not of primary importance. Architecture is.*

*IT system (not your desktop experiments) is made of components. Each component has a well-defined interface and set of operational requirements. E.g. size, speed, security, and such. If your component does meet operational requirements I really do not care if you have written it in Quick Basic, Julia, Rust, Elm, Awk, Bash Script or C++.*

*Just after that, it will be rigorously tested for meeting the functional requirements.*

*Also please do not (ever) forget the economy of IT systems: how much money will it take to develop it and more importantly how much money will it take to maintain what you did, and for the next 10+ years.* -- DBJ 2020 Dec.

This is the collection of my top level C++ headers. Meaning they are or should be used in all of my C++ code. But why do I develop C++ code as I do?

# The Reality and consequences

Operational requirements are economy and market driven.

## Run Time 

- Windows
  - no point in avoiding Win32
  - Win32 in (undefined, C89 like) C API
  - SEH is a reality and intrinsic to Windows
    -  patented M$FT technology
-  Legacy is a big problem
   - UTF-16

## Compile Time

> What compiler? clang-cl.exe 
And why:

- CL 
  - heavy burden of legacy
- clang-cl
  -  much better untroubled and legacy unburdened compiler
- MS STL
  - works rather well with c++ exceptions switched off
  - That is a game changer
  - Makes me free to **avoid the technical debt** ammased in the std lib
    -  iostreams
    -  exceptions
    -  RTTI
    -  new/delete
- C11/17
  - officaly are in cl.exe but one can not mix C/C++ in cpp files using cl.exe
  - clang-cl has no issues, it "just works"
    - clang uses some C++ machinery available in compiler while the code is C.
      - same as in GCC that is done throught `__attribute__((att name here))`
    - I can and will rely on modern C costructs and features 
    - like for example VLA typedefs
    ```cpp
    typedef char (*arr_of_3_char_ptr)[3] ;

    arr_of_3_char_ptr a3p = 
        (arr_of_3_char_ptr)
           malloc( sizeof arr_of_3_char_ptr);
    ```
    - or arrays as arguments
    ```cpp
    void use_buff ( int size, char buff[size]);
    ```

## Do not develop it if it exist

- generaly yes but
  - that requires time for search and testing
  - sometimes so much time it is more feasible to develop it in house

## Note on[ Orthodox C++](https://gist.github.com/bkaradzic/2e39896bc7d8c34e042b)

While I agree in principle I do not agree if I smell the [dogmatic](https://en.wikipedia.org/wiki/Dogma) approach.