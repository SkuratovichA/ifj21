%%% Nepovinná šablona pro projekty IEL %%%
%%% Verze: 2019/01 %%%
\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{fitiel}[2021/10/01 IFJ project]

% základní třída, odvozujeme ze třídy article
\LoadClass[a4paper,11pt]{article}

% nastavení parametrů
\newif\ifczech \let\ifczech\iftrue
\newif\ifslovak \let\ifslovak\iffalse
\newif\ifenglish \let\ifenglish\iffalse

\DeclareOption{english}{\let\ifenglish\iftrue\let\ifczech\iffalse}
\DeclareOption{slovak}{\let\ifslovak\iftrue\let\ifczech\iffalse}
\DeclareOption*{}
\ProcessOptions\relax

% odstranění čísla částí (sections) z nadpisů
\renewcommand{\@seccntformat}[1]{}

%% základní balíčky - šablona
\RequirePackage{ifthen}

%% základní balíčky - dokument
\RequirePackage[left=2cm,text={17cm, 24cm},top=3cm]{geometry}
\RequirePackage{graphicx}
\RequirePackage{float}

% utf-8
\RequirePackage[T1]{fontenc}
\RequirePackage[utf8]{inputenc}

% matematika
\RequirePackage{amsmath}

% klikatelné odkazy (sekce v obsahu)
\RequirePackage[unicode]{hyperref}

% kreslení obvodů 
\usepackage[european,straightvoltages,betterproportions,EFvoltages]{circuitikz}

% jednotky
\usepackage{siunitx}
%%

%% logo a nastavení správného jazyka pro dokument
% čeština
\ifczech
    \usepackage[czech]{babel}
    \newcommand{\logo}{
        \includegraphics[scale=0.1,keepaspectratio]{fig/logo_cz.png}
    }
\else
\fi

% slovenština
\ifslovak
    \usepackage[slovak]{babel}
    \newcommand{\logo}{
        \includegraphics[scale=0.1,keepaspectratio]{fig/logo_cz.png}
    }
\else
\fi

% angličtina
\ifenglish
    \usepackage[english]{babel}
    \newcommand{\logo}{
        \includegraphics[scale=0.1,keepaspectratio]{fig/logo_en.png}
    }
\else
\fi
