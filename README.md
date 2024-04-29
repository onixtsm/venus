# VENUS project

This is the codebase for group 29 (cbl, Q4, TU/e, EE departament). 
### **IF SOMEBODY FINDS IT, IT IS NOT PERMITED TO COPY OR ENSPIRE FROM IT**
## Instalation and first make
I assume that you know how to use git and github. If not follow this [link](./README.md#how-to-contribute-to-the-project), where I give brief explanation.
```bash
git clone 
cd venus
make # Add -j 4 to show how many threads to use for compiling. Greatly speeds up process
./build/rover # run rover.c file. Might also be experiment. See Project layout for more infomation
```
As library provided by university is in this project you do not need to download/install it separately. All libraries provided by university will be compiled at first make.
## Poject layout
```bash
.
├── build
├── external
├── fonts
├── library
├── LICENSE
├── Makefile
├── platform
├── README.md
└── src             # Source directory
    ├── experiments # Testing programs
    ├── libs        # Shared dependencies written by us
    └── rover.c     # Program to run on rover
```
### ./src
To add your code to this project you should edit `src` directory. It contaitns `rover.c` file which is compiled to be run on robot itself. Later we might add similar file for server or something else.
If you want to test out any kind of sensor without affecting whole project, you can use `./src/experiments` directory. Just create any `.c` file (e.g. `foo.c`) and compile it with `make experiments` or `make exp`. These two commands will compile all experiments, so please keep experiments compilable (or use `make -i exp` to continue on compilation failure). To run experiment use `./build/foo`
Also after you are done use `make indent` to indent all code in the same way. It uses `.clang-format` file present at the root of the project.

P.S. If you do not like my formating options it is kinda your problem

### RECAP ON EXPERIMENTS
```bash
vim ./src/experiments/foo.c #edit .c file
make exp
./build/foo # run it
```
Any `.c` files that also have corresponding `.h` files add to `./src/libs/` directory.

### Other stuff
#### Fonts
Fonts for the display (mby you need them) are located in `./fonts` directory.
#### Library
In `./library` are located `libpynq` files.
#### Makefile and .gitignore
Please, keep out any compiled stuff from git repo. `.gitignore` takes care of that, but if you modified compilation process, then update `.gitignore` as well.
`Makefile` is cearfully crafted by me to give best experience for fast work. Modify it if you **know** what you do.
## How to contribute to the project?
For that you need github account.
 1. You need to do steps showed in Instalation section.
 1. Then you need to go to github page of this project and fork it (button on right side). The github will prompt you creating same project on you account.
 1. You need to change origin of your project that you cloned. For that use these commands:
 ```bash
 cd venus
 git remote rm origin # Removing origin pointing to my page
 git set origin <link_to_your_github_page> # setting your page as remote origin
 ```
 4. Use `git add .` or `git add [specific files you changed]` that you want to commit to project.
 1. Then use `git commit -m "Descriptive messige which explains changes you made"`.
 1. Use `git push` to save it on *your* github page. (The very first time it may fail and ask to use `git push --set-upstream origin` (or something like this), then just use it)
 1. Go to you github page. There you will see that github advices you to create pull request. Just make it and then I will be able to review it and give my feedback.

**NOTE**
If you have set your project already and want newest changes from my branch you need to set up 2nd remote origin (ask me how to do that).

## End
For now it looks like all what I wanted to say. When you find a typo on this page then, fix it.
