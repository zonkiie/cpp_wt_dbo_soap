#include <Wt/Dbo/Dbo.h>
#include <Wt/Dbo/backend/Sqlite3.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>
#include <string>
#include <vector>
#include <list>

namespace dbo = Wt::Dbo;
using std::string;
using std::list;
using std::vector;
using namespace boost::uuids;

class User;
class Post;

/*****
 * Dbo tutorial section 2. Mapping a single class
 *****/

class User {
public:
	string id;
	string name;
	string password;
	Wt::WDateTime ctime;
	dbo::collection< dbo::ptr<Post> > posts;

	template<class Action>
	void persist(Action& a)
	{
		dbo::id		(a, id, 		"id");
		dbo::field	(a,	name,		"name");
		dbo::field	(a,	password,	"password");
		dbo::field	(a,	ctime,		"ctime");
		dbo::hasMany(a,	posts, dbo::ManyToOne, "user");
	}
};

class Post
{
public:
	string id;
	string title;
	string body;
	Wt::WDateTime ctime;
	string owner_id;
	dbo::ptr<User> user;
    template<class Action>
    void persist(Action& a)
    {
		dbo::id		(a,	id,		"id");
		dbo::field	(a,	title,	"title");
		dbo::field	(a,	body, 	"body");
		dbo::field	(a,	ctime,	"ctime");
        dbo::belongsTo(a,	user, "user");
    }
};

const char * uuid_cstr()
{
	random_generator gen;
	uuid id = gen();
	string s = to_string(id);
	return s.c_str();
}

namespace Wt {
	namespace Dbo {

		template<>
		struct dbo_traits<User> : public dbo_default_traits {
			typedef std::string IdType;

			static IdType invalidId() {
				return std::string();
			}

			static const char *surrogateIdField() { return 0; }
		};

		template<>
		struct dbo_traits<Post> : public dbo_default_traits {
			typedef std::string IdType;

			static IdType invalidId() {
				return std::string();
			}

			static const char *surrogateIdField() { return 0; }
		};

	}
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
	sqlite3->setProperty("show-queries", "true");
	dbo::Session session;
	session.setConnection(std::move(sqlite3));

	session.mapClass<User>("user");
	session.mapClass<Post>("post");

	/*
	* Try to create the schema (will fail if already exists).
	*/
	session.createTables();

	{
		dbo::Transaction transaction(session);

		std::unique_ptr<User> user{new User()};
		user->name = "Joe";
		user->password = "Secret";
		user->role = Role::Visitor;
		user->karma = 13;

		dbo::ptr<User> userPtr = session.add(std::move(user));
	}

	/*****
	* Dbo tutorial section 4. Querying objects
	*****/

	{
		dbo::Transaction transaction(session);

		dbo::ptr<User> joe = session.find<User>().where("name = ?").bind("Joe");

		std::cerr << "Joe has karma: " << joe->karma << std::endl;

		dbo::ptr<User> joe2 = session.query< dbo::ptr<User> >("select u from user u").where("name = ?").bind("Joe");

		int count = session.query<int>("select count(1) from user").where("name = ?").bind("Joe");
	}

	{
		dbo::Transaction transaction(session);

		typedef dbo::collection< dbo::ptr<User> > Users;

		Users users = session.find<User>();

		std::cerr << "We have " << users.size() << " users:" << std::endl;

		for (const dbo::ptr<User> &user : users)
			std::cerr << " user " << user->name << " with karma of " << user->karma << std::endl;
	}

	/*****
	* Dbo tutorial section 5. Updating objects
	*****/

	{
		dbo::Transaction transaction(session);

		dbo::ptr<User> joe = session.find<User>().where("name = ?").bind("Joe");

		joe.modify()->karma++;
		joe.modify()->password = "public";
	}

	{
		dbo::Transaction transaction(session);
		dbo::ptr<User> joe = session.find<User>().where("name = ?").bind("Joe");
		if (joe)
		joe.remove();
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
