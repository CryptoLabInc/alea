# ALEA

<img src="https://c.pxhere.com/photos/4d/71/dice_die_roll_game_rolling-1085199.jpg!d" srcset="https://c.pxhere.com/photos/4d/71/dice_die_roll_game_rolling-1085199.jpg!d" alt="game, recreation, board game, sports, roll, rolling, die, games, dice, indoor games and sports, tabletop game, Free Images In PxHere">

**ālĕa**: Latin noun meaning a *die*, as in

> ālea iacta est ("The die has been cast") --- Julius Caesar

ALEA is a lightweight, pure C library that provides [CSPRNG](https://en.wikipedia.org/wiki/Cryptographically_secure_pseudorandom_number_generator) functionalties. It is primarily designed for use in cryptographic primitives within lattice-based cryptographic algorithms, including the fully homomorphic encryption (FHE) and the post-quantum cryptography (PQC).

## How to Build

ALEA is a CMake project. You can build the entire project with
```sh
cmake -S . -B $ALEA_BUILD_DIR && cmake --build $ALEA_BUILD_DIR
```
You should supply `$ALEA_BUILD_DIR` for your preference.

### Build Options

| Build Options         | What it is                                                 | Default     |
| --------------------- | ---------------------------------------------------------- | ----------- |
| `BUILD_SHARED_LIBS`   | Build a shared library instead of a static one             | `OFF`       |

## How to Test

You can use [ctest](https://cmake.org/cmake/help/latest/manual/ctest.1.html) to do test runs for the compiled projects. Please use the following command.
```sh
ctest --test-dir $ALEA_BUILD_DIR
```

## License
deb is licensed under the Apache License 2.0, which means that you are free to get and use it for commercial and non-commercial purposes as long as you fulfill its conditions.

See the LICENSE file for more details.
