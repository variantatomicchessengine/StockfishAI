
mkdir("psrc");
mkdir("psrc/syzygy");

processdir("asrc","psrc");
processdir("asrc\\syzygy","psrc\\syzygy");

sub processdir
{
	my $inpath=shift;
	my $outpath=shift;
	opendir(DIR,$inpath);
	while($dir=readdir(DIR))
	{
		if(($dir=~/\.[^\.]+$/)||($dir eq "Makefile")&&(!($dir=~/\.dll$/i)))
		{	
			print "processing $dir ... ";

			open(INF,"$inpath\\$dir");
			@inf=<INF>;
			close(INF);

			my $content=join("",@inf);
			
			my $pcontent=applypatches($dir,$content);

			if(-e "$outpath\\$dir")
			{
				open(OLD,"$outpath\\$dir");
				my $old=join("",<OLD>);
				if($old eq $pcontent)
				{
					$overwrite=0;
				}
			}

			if($overwrite)
			{
				print "applying patch\n";

				open(OUTF,">$outpath\\$dir");
				print OUTF $pcontent;
				close(OUTF);
			}
			else
			{
				print "up to date\n";
			}
		}
	}
}

sub applypatches
{
	my $dir=shift;
	my $content=shift;

	$overwrite=0;

	if($dir eq "position.cpp")
	{
		$content=~s/npm .= npm/npm += 3 * npm \/ 2/;

		$overwrite=1;
	}

	if($dir eq "evaluate.cpp")
	{
		$content=~s/mobility.Us. .= (MobilityBonus.Pt..mob.)/mobility[Us] = 11 * $1 \/ 3/;
		$content=~s/ebonus .= relative_rank.Us, s. . 5 . rr/ebonus += relative_rank(Us, s) * 9 * rr \/ 2/;

		$overwrite=1;
	}

	if($dir eq "types.h")
	{
		# Atomkraft piece values

  		$content=~s/PawnValueMg\s*=\s*[0-9]+/PawnValueMg = 200/;
  		$content=~s/PawnValueEg\s*=\s*[0-9]+/PawnValueEg = 300/;
  		$content=~s/KnightValueMg\s*=\s*[0-9]+/KnightValueMg = 300/;
  		$content=~s/KnightValueEg\s*=\s*[0-9]+/KnightValueEg = 350/;
  		$content=~s/BishopValueMg\s*=\s*[0-9]+/BishopValueMg = 300/;
  		$content=~s/BishopValueEg\s*=\s*[0-9]+/BishopValueEg = 320/;
  		$content=~s/RookValueMg\s*=\s*[0-9]+/RookValueMg = 600/;
  		$content=~s/RookValueEg\s*=\s*[0-9]+/RookValueEg = 800/;
  		$content=~s/QueenValueMg\s*=\s*[0-9]+/QueenValueMg = 1300/;
  		$content=~s/QueenValueEg\s*=\s*[0-9]+/QueenValueEg = 1600/;

  		$overwrite=1;
	}

	if($dir eq "timeman.h")
	{

		if($content=~/(void init\(([^\)]+)\);)/)
		{
			my $func=$1;
			my $param=$2;

			my $repl=<<EOT;
$func
void init_old($param);
EOT
			$content=~s/\Q$func\E/$repl/;
		}

		$overwrite=1;

	}

	if($dir eq "timeman.cpp")
	{
		if($content=~/(void TimeManagement::init\(([^\)]+)\))/)
		{
			my $func=$1;
			my $param=$2;

			my $repl=<<EOT;
$func
{
	TimeManagement::init_old(limits,us,ply);
	if(ply<2)
	{
		optimumTime=limits.time[us]/10;
		if(optimumTime<12000) optimumTime=12000;
		if((optimumTime*3)>limits.time[us]) optimumTime=limits.time[us]/3;
		if(optimumTime>45000) optimumTime=45000;
		maximumTime=optimumTime+2000;
	}
}
void TimeManagement::init_old($param)
EOT
			$content=~s/\Q$func\E/$repl/;
		}

		$overwrite=1;
	}

	return $content;
}