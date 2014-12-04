#! /usr/bin/perl -w
#######################################################################
#
#  This script parses a formatted checkpoint file produced by QChem
#  and checks to see if the shell types need to be switched from 
#  pure to cartesian, or vice versa.
#
#  2014 A. Gilbert
#
#######################################################################

  if (!@ARGV) { die "usage:  fixPureCart.pl input.fchk > output.fchk "; }

  $file = $ARGV[0];

  open (FILE, "<$file") || die "Failed to open file $file";

  while (<FILE>) {
     if (m/Number of basis functions/) {
        @tmp = split(/\s+/, $_);
        $nbasis = $tmp[-1];
        print $_;
     }elsif (m/Shell types/) {
        @types  = ();
        $readShellTypes = 1;
        print $_;
     }elsif (m/Number of primitives/) {
        $readShellTypes = 0;
        if (basisFunctions(@types) != $nbasis) {
           @types = switchTypes(@types);
           if (basisFunctions(@types) != $nbasis) {
              die "Switching pure/cart did not fix things";
           }
        }
        printTypes(@types);
        print $_;
     }elsif ($readShellTypes) {
        $_ =~ s/^\s+|\s+$//g;
        push(@types, split(/\s+/, $_));
     }else {
        print $_;
     }
  }

  close (FILE);

  #--------------------------------------------------------------------

  sub printTypes
  {
      my (@types) = @_;
      my $i = 0;
      while (@types) {
         $n = @types > 6 ? 6 : @types;
         for $i (1..$n) {
             printf("%12d", shift(@types));
         }
         print "\n";
      }
  }


  sub basisFunctions
  {
      my $n = 0;

      for $t (@_){
          if    ($t ==  0) {
             $n += 1;
          }elsif($t ==  1) {
             $n += 3;
          }elsif($t == -1) {
             $n += 4;
          }elsif($t ==  2) {
             $n += 6;
          }elsif($t == -2) {
             $n += 5;
          }elsif($t ==  3) {
             $n += 10;
          }elsif($t == -3) {
             $n += 7;
          }else {
             die "Unsupported angular momentum: ", $t;
          }
      }

      return $n;
  }


  sub switchTypes 
  {
      my @newTypes = ();
      for $t (@_){
          if (abs($t) > 1) { $t = -$t; }
          push(@newTypes, $t);
      }

      return @newTypes;
   }
