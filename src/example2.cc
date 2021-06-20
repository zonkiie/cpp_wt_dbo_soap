#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/backend/Sqlite3.h>
#include <Wt/Dbo/Json.h>
#include <Wt/WDateTime.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>
#include <optional>
#include <string>
#include <vector>
#include <list>
#include <ctime>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>

namespace dbo = Wt::Dbo;
using std::string;
using std::ostringstream;
using std::list;
using std::vector;
using std::optional;
using std::cin;
using std::cout;
using std::cerr;
using std::to_string;
using std::setw;
using std::setfill;
using std::endl;
using std::ios_base;
using namespace boost::uuids;
using Wt::WDateTime;
using dbo::JsonSerializer;

class User;
class Post;
string current_timestamp_string();
string timestamp_to_string(tm *ltm);
const char * uuid_cstr();
string uuid_str();


namespace Wt {
	namespace Dbo {

		template<>
		struct dbo_traits<User> : public dbo_default_traits {
			typedef std::string IdType;

			static IdType invalidId() { return std::string(); }
			static const char *surrogateIdField() { return 0; }
            static const char *versionField() { return 0; }
		};

		template<>
		struct dbo_traits<Post> : public dbo_default_traits {
			typedef std::string IdType;

			static IdType invalidId() { return std::string(); }
			static const char *surrogateIdField() { return 0; }
            static const char *versionField() { return 0;}
		};
	}
}

class CreateAble
{
public:
	optional<string> createTableString() { return {}; }
};

string current_timestamp_string()
{
	time_t now = time(0);
	tm *ltm = localtime(&now);
	return timestamp_to_string(ltm);
}

string timestamp_to_string(tm *ltm)
{
	//return string(to_string(1900 + ltm->tm_year) + "-" + to_string(1 + ltm->tm_mon) + "-" + to_string(ltm->tm_mday) + " " + to_string(ltm->tm_hour) + ":" + to_string(ltm->tm_min) + ":" + to_string(ltm->tm_sec));
	ostringstream ostr;
	ostr << setw(4) << setfill('0') << 1900 + ltm->tm_year << setw(1) << "-" << setw(2) << 1 + ltm->tm_mon << setw(1) << "-" << setw(2) << ltm->tm_mday << setw(1) << " " << setw(2) << ltm->tm_hour << setw(1) << ":" << setw(2) << ltm->tm_min << setw(1) << ":" << setw(2) << ltm->tm_sec;
	return ostr.str();
}

/*****
 * Dbo tutorial section 2. Mapping a single class
 *****/

class User: public CreateAble
{
public:
	string id;
	string name;
	string password;
	string ctime;
	dbo::collection< dbo::ptr<Post> > posts;
	list<Post> postsList;

	template<class Action>
	void persist(Action& a)
	{
		dbo::id		(a, id, 		"id", 36);
		dbo::field	(a,	name,		"name");
		dbo::field	(a,	password,	"password");
		dbo::field	(a,	ctime,		"ctime");
		dbo::hasMany(a,	posts, dbo::ManyToOne, "user");
		if(a.getsValue()) {
			//cerr << "User.getsValue()" << endl;
		}
		if(a.setsValue()) {
			//cerr << "User.setsValue()" << endl;
			createPostList();
		}
	}
	
	static optional<string> createTableString()
	{
		return "CREATE TABLE IF NOT EXISTS \"user\" (id text NOT NULL PRIMARY KEY, name text NOT NULL, password text NOT NULL, ctime timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP);";
	}
	
	User(): id(uuid_str()) {}
	
	void createPostList()
	{
		typedef dbo::collection< dbo::ptr<Post> > Posts;
		for (Posts::const_iterator i = posts.begin(); i != posts.end(); ++i){
			//postsList.push_back(**i);
		}
		
	}
};

class Post: public CreateAble
{
public:
	string id;
	string title;
	string body;
	string ctime;
	string user_id;
	dbo::ptr<User> user;
    template<class Action>
    void persist(Action& a)
    {
		dbo::id		(a,	id,		"id", 36);
		dbo::field	(a,	title,	"title");
		dbo::field	(a,	body, 	"body");
		dbo::field	(a,	ctime,	"ctime");
        dbo::belongsTo(a,	user, "user");
		if(a.getsValue()) {
			//cerr << "Post.getsValue()" << endl;
		}
		if(a.setsValue()) {
			//cerr << "Post.setsValue()" << endl;
		}
    }
    
	static optional<string> createTableString()
	{
		return "CREATE TABLE IF NOT EXISTS \"post\" (id text NOT NULL PRIMARY KEY, title text NOT NULL, body text, user_id text NOT NULL, ctime TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP, CONSTRAINT \"fk_post_user\" FOREIGN KEY (\"user_id\") REFERENCES \"user\" (\"id\") ON DELETE CASCADE DEFERRABLE INITIALLY DEFERRED);";
	}
	Post(): id(uuid_str()) {}
};

std::ostream& operator<<(std::ostream& os, Post const& post)
{
	os << "{id:" << post.id << ", Title:" << post.title << ", Body:" << post.body << "}";
	return os;
}

std::ostream& operator<<(std::ostream& os, User const& user)
{
	os << "{id:" << user.id << ", name:" << user.name << ", posts:[";
	typedef dbo::collection< dbo::ptr<Post> > Posts;
	for (Posts::const_iterator i = user.posts.begin(); i != user.posts.end(); ++i){
			os << **i << ",";
		}
	os << "]" << endl;
	return os;
}


const char * uuid_cstr()
{
	random_generator gen;
	uuid id = gen();
	string s = to_string(id);
	return s.c_str();
}

string uuid_str()
{
	random_generator gen;
	uuid id = gen();
	return to_string(id);
}


void run()
{
	/*****
	* Dbo tutorial section 3. A first session
	*****/

	/*
	* Setup a session, would typically be done once at application startup.
	*
	* For testing, we'll be using Sqlite3's special :memory: database. You
	* can replace this with an actual filename for actual persistence.
	*/
	std::unique_ptr<dbo::backend::Sqlite3> sqlite3(new dbo::backend::Sqlite3(":memory:"));
	//sqlite3->setProperty("show-queries", "true");
	dbo::Session session;
	session.setConnection(std::move(sqlite3));

	session.mapClass<User>("user");
	session.mapClass<Post>("post");

	/*
	* Try to create the schema (will fail if already exists).
	*/
	//session.createTables();
	{
		dbo::Transaction transaction(session);
		session.execute(User::createTableString().value());
		session.execute(Post::createTableString().value());
	}
	
	{
		dbo::Transaction transaction(session);

		std::unique_ptr<User> user{new User()};
		user->name = "Joe";
		user->password = "Secret";
		user->ctime = current_timestamp_string();
		//user->ctime = WDateTime::currentDateTime();
		std::unique_ptr<Post> post{new Post()};
		post->title = "This is the Title!";
		post->body = "This is the Body!";
		post->ctime = current_timestamp_string();
		dbo::ptr<User> userPtr = session.add(std::move(user));
		post->user = userPtr;
		dbo::ptr<Post> postPtr = session.add(std::move(post));
		
	}

	/*****
	* Dbo tutorial section 4. Querying objects
	*****/

	{
		dbo::Transaction transaction(session);

		dbo::ptr<User> joe = session.find<User>().where("name = ?").bind("Joe");

		std::cerr << "Joe has ctime: " << joe->ctime << std::endl;
		
		cerr << "Fetching joe2\n";

		dbo::ptr<User> joe2 = session.query< dbo::ptr<User> >("select u from user u").where("name = ?").bind("Joe");
		
		cerr << "Joe 2 fetched.\n";

		int count = session.query<int>("select count(1) from user").where("name = ?").bind("Joe");
	}
	cerr << "Fetch\n";

	{
		dbo::Transaction transaction(session);

		typedef dbo::collection< dbo::ptr<User> > Users;

		Users users = session.find<User>();

		/*std::cerr << "We have " << users.size() << " users:" << std::endl;

		// Example for simple serializing via << operator
		for (const dbo::ptr<User> &user : users)
			//std::cerr << " user " << user->name << " with ctime " << user->ctime << std::endl;
			std::cerr << *user << endl;
		*/
		// Built-in Serializer
		JsonSerializer jsz(cout);
		jsz.serialize(users);
		
		
	}

	/*****
	* Dbo tutorial section 5. Updating objects
	*****/

	{
		dbo::Transaction transaction(session);

		dbo::ptr<User> joe = session.find<User>().where("name = ?").bind("Joe");

		joe.modify()->password = "public";
	}

	{
		dbo::Transaction transaction(session);
		dbo::ptr<User> joe = session.find<User>().where("name = ?").bind("Joe");
		if (joe) joe.remove();
	}

	{
		dbo::Transaction transaction(session);

		dbo::ptr<User> silly = session.add(std::unique_ptr<User>{new User()});
		silly.modify()->name = "Silly";
		silly.remove();
	}

}

int main(int argc, char **argv)
{
	run();
}
