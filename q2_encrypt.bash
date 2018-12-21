#!/bin/bash

small=`head -n1 $1` #Extracting small letters
capital=`tail -n1 $1` #Extracting capital letters
code=`echo $small$capital` #Concatenating all letters
letters=abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ #Ordered letters
tr [$letters] [$code] <  $2  #Encrypting message file input

