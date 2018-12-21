#!/bin/bash

small=`head -n1 $1` #Extracting small letters
capital=`tail -n1 $1` #Extracting big letters
code=`echo $small$capital` #Concatenating all letters
letters=abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ #Ordered letters
tr [$code] [$letters] <  $2  #Decrypting message input
