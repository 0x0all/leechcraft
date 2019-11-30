/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#ifndef PLUGINS_LACKMAN_DEPTREEBUILDER_H
#define PLUGINS_LACKMAN_DEPTREEBUILDER_H
#include <memory>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/properties.hpp>
#include <QHash>
#include <QStack>
#include "repoinfo.h"

namespace LC
{
namespace LackMan
{
	struct CycleDetector;
	struct FulfillableChecker;
	struct VertexPredicate;

	class DepTreeBuilder final
	{
		struct VertexInfo
		{
			/** Makes sense only for those vertices representing
				* packages, not any-typed dependencies, like
				* interfaces.
				*/
			int PackageId_ = -1;

			/** Makes sense only for vertices representing any-
				* typed dependencies, like interfaces.
				*/
			QString Dependency_;

			bool IsFulfilled_ = false;

			/** Wheter this graph vertex is considered to be
				* "fulfilled" when any of its dependencies is
				* "fulfilled" or when all dependencies are
				* fulfilled.
				*
				* Dependencies are represented by ChildVertices_,
				* so basically TAny-typed node's fulfilled field
				* is calculated as OR, and TAll — as AND.
				*
				* Generally, TAll-vertices are used to represent a
				* plugin — a plugin may be installed if all its
				* dependencies may be installed. TAny-vertices are
				* used to represent a dependency on interface:
				* such a dependency is fulfilled when any plugin
				* which implements that interface may be installed.
				*/
			enum Type
			{
				TAny,//!< TAny It's enough for any child vertex to be fulfilled.
				TAll //!< TAll All child vertices should be fulfilled.
			} Type_ = TAny;

			/** Constructs an empty VertexInfo of type TAny.
				*/
			VertexInfo () = default;

			/** @brief Constructs vertex with type TAll and
				* given packageId used to identify this vertex.
				*
				* packageId may be later used to identify the
				* package this vertex represents in the dependency
				* tree.
				*
				* @param[in] packageId ID of the package this
				* vertex represents.
				*/
			VertexInfo (int packageId);

			/** @brief Constructs vertex with type TAny and
				* given dependency name used to identify this
				* vertex.
				*
				* depName may be later used to identify the
				* dependency name (like name of the interface)
				* this vertex represents in the dependency tree.
				*
				* @param[in] depName Name of the dependency this
				* vertex represents.
				*/
			VertexInfo (const QString& depName);
		};

		typedef std::shared_ptr<VertexInfo> VertexInfo_ptr;

		typedef boost::property<boost::vertex_color_t, boost::default_color_type,
				VertexInfo> VertexProperty;
		typedef boost::adjacency_list<boost::vecS, boost::vecS,
				boost::bidirectionalS, VertexProperty> Graph_t;

		typedef Graph_t::vertex_descriptor Vertex_t;
		typedef Graph_t::edge_descriptor Edge_t;
		typedef Graph_t::out_edge_iterator OutEdgeIterator_t;
		typedef Graph_t::in_edge_iterator InEdgeIterator_t;

		QHash<int, Vertex_t> Package2Vertex_;
		QHash<Dependency, Vertex_t> Dependency2Vertex_;

		typedef QMap<Edge_t, QPair<Vertex_t, Vertex_t>> Edge2Vertices_t;
		Edge2Vertices_t Edge2Vertices_;

		Graph_t Graph_;

		friend struct CycleDetector;
		friend struct FulfillableChecker;
		friend struct VertexPredicate;

		QList<int> PackagesToInstall_;
	public:
		DepTreeBuilder (int);

		/** Whether it is possible to install the package for
			* which this graph is being built.
			*/
		bool IsFulfilled () const;

		/** Returns the list of the top-level unfulfilled
			* dependencies.
			*/
		QStringList GetUnfulfilled () const;

		/** @brief Returns the list of packages that need to be
			* installed in order to install the package passed to
			* the constructor.
			*
			* The return value of this function only makes sense if
			* the package may be installed at all: if IsFulfilled()
			* returns true.
			*
			* @return The list of packages to be installed.
			*/
		const QList<int>& GetPackagesToInstall () const;
	private:
		/** @brief Builds the part of dependency tree for the
			* package identified by package.
			*
			* Once this function has finished building the layers
			* of dep tree relevant to the package lpi, it would
			* recursively call itself on those packages on which
			* package depends.
			*
			* @param[in] package The ID of the package for which
			* to build the
			* tree.
			*/
		void InnerLoop (int package);
	};
}
}

#endif
