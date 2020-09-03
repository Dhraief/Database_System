	// This file requires at least C99 to compile

	/**
	 * @file   db.c
	 * @author Merlin Nimier-David <merlin.nimier-david@epfl.ch>
	 * @author Jean-C�dric Chappelier <jean-cedric.chappelier@epfl.ch>
	 *
	 * @copyright EPFL 2020
	 **/
	/**
	 * @section DESCRIPTION
	 *
	 * Template du homework du cours CS-207, ann�e 2020.
	 **/

	#include <stdio.h>
	#include <stdlib.h> // EXIT_SUCCESS/FAILURE
	#include <math.h>   // fabs()
	#include <string.h> // memset()
	#include <stdint.h> // uint32_t
	#include <inttypes.h> // PRIu32 & SCNu32

	// ----------------------------------------------
	//   ___             _            _
	//  / __|___ _ _  __| |_ __ _ _ _| |_ ___
	// | (__/ _ \ ' \(_-<  _/ _` | ' \  _(_-<
	//  \___\___/_||_/__/\__\__,_|_||_\__/__/

	#define DB_MAX_SIZE    20u
	#define QUERY_MAX_SIZE  5u

	// ----------------------------------------------
	//  _____
	// |_   _|  _ _ __  ___ ___
	//   | || || | '_ \/ -_|_-<
	//   |_| \_, | .__/\___/__/
	//       |__/|_|

	/* D�finissez ici les types demand�s :
	*    StudentKind,
	*    SCIPER,
	*    Student,
	*    Database,
	* et QueryResult.
	*/
	typedef enum {
			Bachelor, Master, Exchange
	} StudentKind;
	int const StudentKindCount = (int) Exchange + 1;
	typedef unsigned long int SCIPER;

	typedef struct Student {
		SCIPER sciper;
		float grade_sn, grade_hw, grade_exam;
		StudentKind type;
		const struct Student* teammate;
	} Student;
	typedef Student Database[DB_MAX_SIZE];
	typedef const Student* QueryResult[QUERY_MAX_SIZE];

	// --------------------------------------
	//-------------------PROTOTYPES -----------

	// ----------------------------------------------
	//   ___               _
	//  / _ \ _  _ ___ _ _(_)___ ___
	// | (_) | || / -_) '_| / -_|_-<
	//  \__\_\\_,_\___|_| |_\___/__/

	size_t db_entry_count(const Database db) {
		int i = 0;
		for (i = 0; i < DB_MAX_SIZE; i++) {
			if (db[i].sciper == 0)
				return i;
		}
		return i;
	}

	// ----------------------------------------------

	//----------------------------------------------
	const Student* get_student_by_sciper(const Database db, SCIPER sciper) {
		const Student* res = malloc(sizeof(Student));
		int l=db_entry_count(db);
		for (int i = 0; i < l; i++) {
			if (db[i].sciper == sciper) {
				res = &db[i];
				return res;
			}
		}
		return NULL;
	}
	//----------------------------------------------
	void get_students_by_type(const Database db, StudentKind type,
			QueryResult result_out) {
		int n = 0;
		int l=db_entry_count(db);
		for (int i = 0; i < l; i++) {
			if (db[i].type == type && n<QUERY_MAX_SIZE) {
				result_out[n] = &db[i];
				n++;
			}
		}
	}
	// ----------------------------------------------

	/*return 0 if not valide and 1 if valid */
	int validTeammate(const Database db, int i) {
		const Student* binome = db[i].teammate;
		//valide
		if (binome->teammate == &db[i])
			return 1;
		//invalide
		else {
			//le binome de l'étudiant n'a pas de binôme
			if (binome->teammate == NULL) {
				fprintf(stderr,
						"%lu  a   %lu comme binôme mais %lu n'a pas de binome \n",
						db[i].sciper, binome->sciper, binome->sciper);
			} else {
				//le binome de l'étudiant a un autre binôme 
				fprintf(stderr,
						"%lu  a   %lu comme binôme mais %lu a %lu comme binôme \n",
						db[i].sciper, binome->sciper, binome->sciper,
						(binome->teammate)->sciper);
			}
			return 0;
		}
	}

	int check_teammates_consistency(const Database db) {
		int res = 1;
		int l=db_entry_count(db);
		for (int i = 0; i < l; i++) {
			// pas de binôme
			if (db[i].teammate == NULL) {
				res *= 1;
			}
			// a un binome
			else {
				res = res * validTeammate(db, i);
			}
		}
		if (res == 1) {
			return 0;
		} else
			return -1;
	}

	// ----------------------------------------------
	double grade_average(const Student* stud) {
		if (stud == NULL)
			return 0.0;
		return 0.1 * stud->grade_hw + 0.4 * stud->grade_sn + 0.5 * stud->grade_exam;
	}

	// ----------------------------------------------
	double team_diff(const Student* stud) {
		return fabs(grade_average(stud) - grade_average(stud->teammate));
	}

	// ----------------------------------------------
	#define student_to_index(tab, student) (size_t)((student) - (tab))

	// ----------------------------------------------

	//donne un tableau contenant les écarts de binômes
	void ecBin(const Database db, double res[DB_MAX_SIZE]) {
		for (int i = 0; i < DB_MAX_SIZE; i++) {
			if (db[i].teammate != NULL) {
				res[i] = team_diff(&db[i]);
			} else {
				res[i] = 0.0;
			}
		}
	}
	//initialise le tableau de pointeurs
	void initialiserEtu(Student* res[DB_MAX_SIZE]) {
		for (int i = 0; i < DB_MAX_SIZE; i++) {
			res[i] = NULL;
		}
	}

	// rempli le tableau avec le pointeur des étudiants faibles sinon NULL

	void remplirEtuF(const Database db, Student* res[DB_MAX_SIZE]) {

		//contient 1 si on a choisit un étudiant du binôme sinon 0
		// Celaa a été ajouté pour le cas où deux étudiants ont la même moyenne

		int indicesBinomes = 0;
		for (int i = 0; i < DB_MAX_SIZE; i++) {
			// a un teammate
			if (db[i].teammate != NULL) {

				if ((grade_average(&db[i]) <= grade_average(db[i].teammate))) {
					indicesBinomes = (int) (student_to_index(db, (db[i].teammate)));
					res[i] = &db[i];
					res[indicesBinomes] = NULL;

				} else {
					res[i] = NULL;
				}
			} else
				res[i] = NULL;
		}
	}

	void swap(double ecartBin[DB_MAX_SIZE], Student* etuFaible[DB_MAX_SIZE], int i,
			int j) {

		double const tmpDouble = ecartBin[i];
		ecartBin[i] = ecartBin[j];
		ecartBin[j] = tmpDouble;

		Student* tmpPtr = etuFaible[i];
		etuFaible[i] = etuFaible[j];
		etuFaible[j] = tmpPtr;
	}

	/* idee: 
	créer un tableau qui contient les écart de binôme: ecBin
	créer un tableau  de pointeurs qui contient un pointeur seulement si l'indice 
	correspond à un étudiant faible :etuFaible
	Trier ecBin et etuFaible selon l'écart
	renvoyer les QUERY_MAX faible de etufaible
	*/
	void get_least_homegenous_teams(const Database db, QueryResult result_out) {
		double ecartBin[DB_MAX_SIZE];
		ecartBin[0] = 0.0;
		ecBin(db, ecartBin); //ecartBin contient les écarts de binoes
		Student* etuFaible[DB_MAX_SIZE];
		initialiserEtu(etuFaible);
		remplirEtuF(db, etuFaible); // contient que les étudiants faibles

		//sort ecartBin croissant
		for (int i = 0; i < DB_MAX_SIZE; i++) {
			for (int j = i + 1; j < (DB_MAX_SIZE - 1); j++) {
				if (ecartBin[i] < ecartBin[j]) {
					/* Exchange two elements */
					swap(ecartBin, etuFaible, i, j);
				}
			}
		}

		int n = 0;
		int size= db_entry_count(db);
		//database plus petite que Query
		if(size<QUERY_MAX_SIZE){
		for(int j = 0 ; j< size;j++){
			if (etuFaible[j] != NULL) {
				result_out[n] = etuFaible[j];
				n++;
			}
		}
		for(int j = n ; j< size;j++){
			result_out[j] = NULL;
		}
		//database plus grande que Query
		}
		else {
				int i=0;
				while ( (i < DB_MAX_SIZE) && (n < QUERY_MAX_SIZE) ) {
					if (etuFaible[i] != NULL) {
						result_out[n] = etuFaible[i];
						n++;
					}
					i++;
				}
		}
		


	}

	// ----------------------------------------------
	//  ___   _____
	// |_ _| / / _ \
	//  | | / / (_) |
	// |___/_/ \___/

	void addPointers(Database db, int tab[DB_MAX_SIZE]) {
		for (int i = 0; i < db_entry_count(db); i++) {
			if (tab[i] == 0)
				db[i].teammate = NULL;
			else
				db[i].teammate = get_student_by_sciper(db, tab[i]);
		}

	}

	int load_database(Database db_out, const char* filename)

	{
		FILE* entree = fopen(filename, "r");

		int sciperBinomes[DB_MAX_SIZE]; //contient les sciper des binomes respectifs
		sciperBinomes[0] = 0;

		if (entree == NULL) {
			fprintf(stderr, "errer dans %s n", filename);
			return -1;
		} else

		{

			int n = 0; //number of students
			int t = -1; //type
			int validIn = 1;
			int sciper = 0;

			validIn = fscanf(entree, "%" SCNu32, &sciper); // read SCIPER;
			float grade_sn = 0;
			float grade_hw = 0;
			float grade_exam = 0;
			StudentKind kind;

			do {

				validIn += fscanf(entree, "%f" SCNu32, &grade_sn); // read sn
				validIn += fscanf(entree, "%f" SCNu32, &grade_hw);// read hw
				validIn+= fscanf(entree, "%f" SCNu32, &grade_exam);// read exa
				
				validIn += fscanf(entree, "%" SCNu32, &t);// read Type
				if ((t < 0 || t > 2)) {
					return -1;
				} else {
					kind = t;
				}

				//sciper binomes
				validIn+= fscanf(entree, "%" SCNu32, &sciperBinomes[n]);// read SCIPER

				if (validIn != 6)
					return -1;

				Student st = { sciper, grade_sn, grade_hw, grade_exam, kind };
				db_out[n] = st;
				n++;

			validIn= fscanf(entree, "%" SCNu32, &sciper); // read SCIPER;

		} while (!feof(entree) && (n < DB_MAX_SIZE));

	}


	addPointers(db_out, sciperBinomes);
	fclose(entree);
	// check for consistency of pointers (pairs)
	return check_teammates_consistency(db_out);
	}
	// ----------------------------------------------
	void fprintf_student_kind(FILE* restrict stream, StudentKind sk) {
	switch (sk) {
	case Bachelor:
		fputs("bachelor", stream);
		break;
	case Master:
		fputs("master  ", stream);
		break;
	case Exchange:
		fputs("exchange", stream);
		break;
	default:
		fputs("unknown ", stream);
		break;
	}
	}

	// ----------------------------------------------
	void write_student(const Student* student, FILE* fp) {
	fprintf(fp, "%07d - %.2f, %.2f, %.2f - ", student->sciper, student->grade_sn,
			student->grade_hw, student->grade_exam);
	fprintf_student_kind(fp, student->type);
	if (student->teammate != NULL) {
	fprintf(fp, " - %06" PRIu32, student->teammate->sciper);
	} else {
	fprintf(fp, " - none");
	}
	fprintf(fp, "\n");
	}

	// ----------------------------------------------
	int write_query_results(QueryResult result, const char* filename) {
	FILE* fp = fopen(filename, "w");
	if (fp == NULL) {
	fprintf(stderr, "Error: could not open file %s for writting: ", filename);
	perror(NULL); // optionnel
	return -1;
	}
	for (size_t i = 0; i < QUERY_MAX_SIZE && result[i] != NULL; ++i) {
	write_student(result[i], fp);
	}
	fclose(fp);
	printf("Query results saved to: %s\n", filename);
	return 0;
	}

	// ----------------------------------------------
	//  __  __      _
	// |  \/  |__ _(_)_ _
	// | |\/| / _` | | ' \
	// |_|  |_\__,_|_|_||_|

	int main(int argc, char** argv) {
	const char* input_filename = "many_teams.txt"; // default input filename
	if (argc >= 2) {
	input_filename = argv[1];
	}

	Database db;
	memset(db, 0, sizeof(db));
	int success = load_database(db, input_filename);
	if (success != 0) {
	fputs("Could not load database.\n", stderr);
	return EXIT_FAILURE;
	}

	// Print contents of database
	puts("+ ----------------- +\n"
		"| Students database |\n"
		"+ ----------------- +");
	const size_t end = db_entry_count(db);
	for (size_t i = 0; i < end; ++i) {
	write_student(&db[i], stdout);
	}

	// Extract students of each kind
	QueryResult res;
	memset(res, 0, sizeof(res));
	char filename[] = "res_type_00.txt";
	const size_t filename_mem_size = strlen(filename) + 1;
	for (StudentKind sk = Bachelor; sk < StudentKindCount; ++sk) {
	get_students_by_type(db, sk, res);
	snprintf(filename, filename_mem_size, "res_type_%02d.txt", sk);
	write_query_results(res, filename);
	}

	// Extract least homogeneous teams
	get_least_homegenous_teams(db, res);


	write_query_results(res, "bad_teams.txt");

	return EXIT_SUCCESS;
	}
