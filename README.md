# CSNEK :: A bot that plays snake

A snake game AI written in C using ncurses.  
The bot uses the A* algorithm to find it's path toward the food.  
More info: [https://en.wikipedia.org/wiki/A*_search_algorithm](wikipedia)  
Can also be played manually but why would you do that?  

This project is very much in development.  

<img src="snek.jpg" width="200" />

## Compile and run

    make

    # run bot
    ./csnek -b -s 2 

## Commandline args

    $ ./csnek -h                                                                                                                             17:29:46
    CSNEK :: A bot that plays snake
    Optional args:
        -H      play the game like a real human! (default)
        -b      let the bot do the work!
        -s      speed in miliseconds inbetween draws (default=100)
        -g      grow amount (default=1)
        -f      amount of food generated (default=1)

## Controls when playing manually

    h, ←    move left
    j, ↓    move down
    k, ↑    move up
    l, →    move right

    <space> toggle pause
    q       quit


![screenshot.png](screenshot.png)

## Bot Info
- [https://en.wikipedia.org/wiki/A*_search_algorithm](https://en.wikipedia.org/wiki/A*_search_algorithm)  
- [https://www.youtube.com/watch?v=-L-WgKMFuhE](https://www.youtube.com/watch?v=-L-WgKMFuhE)  
- [https://johnflux.com/category/nokia-6110-snake-project/](https://johnflux.com/category/nokia-6110-snake-project/)  
- [https://www.youtube.com/watch?v=tjQIO1rqTBE](https://www.youtube.com/watch?v=tjQIO1rqTBE)  
