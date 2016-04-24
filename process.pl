open(LOG,">log.txt");

mkdir("asrc");
mkdir("asrc/syzygy");

my %variants=("KOTH"=>1,"HORDE"=>1,"RACE"=>1,"HOUSE"=>1,"CHESS960"=>1,"ATOMIC"=>1,"STANDARD"=>1,"THREECHECK"=>1,"ANTI"=>1);

processdir("src","asrc");
processdir("src\\syzygy","asrc\\syzygy");

sub processdir
{
	my $inpath=shift;
	my $outpath=shift;
	opendir(DIR,$inpath);
	while($dir=readdir(DIR))
	{
		my $cpp=($dir=~/\.cpp$/);
		my $h=($dir=~/\.h$/);
		my $bar="------------------------";

		if($cpp||$h||($dir eq "Makefile"))
		{	
			print "processing $dir\n";

			printlog("$bar\nprocessing $dir\n$bar\n");

			open(INF,"$inpath\\$dir");
			@inf=<INF>;
			close(INF);

			if($dir ne "Makefile")
			{
			
				my $hasvariant=1;

				my $round=1;

				while($hasvariant)
				{

					@outf=();

					printlog("$bar\nround $round\n$bar\n");

					print "round $round length ",@inf+0,"\n";

					my $varianton=0;
					my $allowedon=0;
					my $iflevel=0;

					$hasvariant=0;

					while(@inf>0)
					{
						my $line=shift(@inf);

						if($line=~/^\s*#/)
						{
							printlog($line);
						}

						if($line=~/^\s*#if/)
						{
							if($varianton)
							{
								$iflevel++;
							}
						}

						if($line=~/^\s*#ifdef ([^\s]+)/)
						{
							my $def=$1;
							if($iflevel==0)
							{							
								if($variants{$def})
								{
									$iflevel=1;
									$varianton=1;
									$hasvariant=1;								
									$allowedon=($def eq "ATOMIC");
								}
								else
								{
									push(@outf,$line);
								}
							}
							else
							{							
								if($allowedon)
								{
									push(@outf,$line);
								}
							}
							
						}
						elsif($line=~/^\s*#ifndef ([^\s]+)/)
						{
							my $ndef=$1;
							if($iflevel==0)
							{							
								if($variants{$ndef})
								{
									$iflevel=1;
									$varianton=1;								
									$hasvariant=1;
									$allowedon=($def ne "ATOMIC");
								}
								else
								{
									push(@outf,$line);
								}
							}
							else
							{
								if($allowedon)
								{
									push(@outf,$line);
								}
							}
						}
						elsif($line=~/^\s*#el/)
						{
							if($varianton)
							{
								if($iflevel==1)
								{
									$allowedon=!$allowedon;
								}
								else
								{
									if($allowedon)
									{
										push(@outf,$line);
									}
								}
							}
							else
							{
								push(@outf,$line);
							}
						}
						elsif($line=~/^\s*#endif/)
						{												
							if($varianton)
							{
								$iflevel--;
								if($iflevel>0)
								{
									if($allowedon)
									{
										push(@outf,$line);
									}
								}
								else
								{
									$varianton=0;
									$allowedon=0;
								}
							}
							else
							{
								push(@outf,$line);
							}
						}
						else
						{
							if($varianton)
							{
								if($allowedon)
								{
									push(@outf,$line);
								}
							}
							else
							{
								push(@outf,$line);
							}
						}
					}

					@inf=@outf;

					$round++;

				}

			}
			else
			{
				@outf=@inf;
			}

			open(OUTF,">$outpath\\$dir");
			my $content=join("",@outf);

			my $pcontent=applypatches($dir,$content);

			print OUTF $pcontent;
			close(OUTF);
		}
	}
}

sub printlog
{
	my $line=shift;
	if($line=~/^\s*#/)
	{
		print LOG "\t",$line;
	}
	else
	{
		print LOG $line;		
	}
}

sub applypatches
{
	my $dir=shift;
	my $content=shift;

	if($dir eq "position.cpp")
	{
		$content=~s/if .is_horde.. && ci.ksq == SQ_NONE.\s*return false;//;
		$content=~s/.. .is_horde.. . wksq != SQ_NONE : ..!is_atomic.. .. wksq != SQ_NONE. && piece_on.wksq. != W_KING..//;
		return $content;
	}

	if($dir eq "Makefile")
	{
		$content=~s/EXE . stockfish/EXE = stockfishai/;
		return $content;
	}

	if($dir eq "misc.cpp")
	{
		$content=~s/ss << "Stockfish "/ss << "StockfishAI "/;
		return $content;
	}

	if($dir eq "ucioption.cpp")
	{
		$content=~s/o."UCI_Atomic".(\s*)<<(\s*)Option.false.;/o["UCI_Atomic"]$1<<$2Option(true);/;
		$content=~s/(std::ostream& operator<<.std::ostream& os, const OptionsMap& om. .)/$1\nreturn os;\n/;
	}

	return $content;
}