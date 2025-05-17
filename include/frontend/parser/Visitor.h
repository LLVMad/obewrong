#ifndef OBW_VISITOR_H
#define OBW_VISITOR_H

// The idea is taken from the book:
//
// Modern C++ Design: Generic Programming and Design Patterns Applied
// By Andrei Alexandrescu
//
// All Credentials go to the Andrei
//
// Please see the Copyright notice in the book
//

// ------------------------------------------------------------------------------------------------------------------------------------------------
// 1. Visitor part

	// You need to derive a Visitor class from VisitorBase. That is mandatory
	class VisitorBase
	{
		public:
			VisitorBase(void) {}
			virtual ~VisitorBase(void) {}
	};


	// Additionally you need to derive your visitor class from this class
	// and as many times as you want to visit specific classes that you must define
	// as template parameters.
	//
	// So a typical visitor class definition looks like
	//
	// class AVisitorForSomeOtherTypes : 	public VisitorBase
	//										public Visitor<VisitibleType1>,
	//										public Visitor<VisitibleType2>,
	//										public Visitor<VisitibleTypex>
	//
	// With such a Visitor you can visit the classes specified in the template parameters
	//
	template <class T>
	class Visitor
	{
		public:
			Visitor(void) {}
			virtual ~Visitor(void) {}

			virtual void visit(T&) = 0;
	};


// ------------------------------------------------------------------------------------------------------------------------------------------------
// 2. Visitable part

	class VisitableBase
	{
		public:
			VisitableBase(void) {}
			virtual ~VisitableBase() {}
			// Will be overridden by macro
			// If somebody forgets to insert the macro, we will do nothing
			virtual void acceptAGuestVisitor(VisitorBase&) { };
		protected:

			// Because the DEFINE_VISITABLE macro will be used to finally call this function
			// Type T, set to "this" of the class where the macro was defined, T will be
			// of the type of the class where the macro was defined
			// This is just a redirector. And it avoids circular forward declarations
			// Hence "Acyclic"

			template <class TypeOfClassThatWillBeVisited>
			static void acceptDecoupled(TypeOfClassThatWillBeVisited& visited, VisitorBase* guestVisitorWhoWillVisitSomeOtherClass)
			{
				// Apply the Acyclic Visitor
				// Since Visitor classes are always derived from VisitorBase
				// and specific Visitor <templated> classes, the dynamic cast will
				// work, if the guest, is also in the list of Base Classes from which
				// the visitor is derived
				//
				// The visit function will not be called for unknown Visitors
				// in the example above, not for a "VisitibleTypekkk"

				if (Visitor<TypeOfClassThatWillBeVisited>* visitorGuest =	dynamic_cast<Visitor< TypeOfClassThatWillBeVisited>*>(guestVisitorWhoWillVisitSomeOtherClass))
				{
					// Call the Visitors visit function (if existing)
					visitorGuest->visit(visited);
				}
				return;
			}
	};


// This Macro hast to be defined in the class that shall be visited.
// So, in the visitable class.
// It will define the accept function which is typical for the visitor pattern.
// Accept will then call the "static acceptDecoupled" function, which
// calls the visit function with "*this" (of the class where the macro was defined)
// as parameter.
#define DEFINE_VISITABLE() virtual void acceptAGuestVisitor(VisitorBase* guestVisitorWhoWillVisitSomeOtherClass) { acceptDecoupled(*this, guestVisitorWhoWillVisitSomeOtherClass); }

#endif
