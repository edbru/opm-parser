/*
  Copyright 2014 Statoil ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef ECLIPSE_GRIDPROPERTIES_HPP_
#define ECLIPSE_GRIDPROPERTIES_HPP_


#include <string>
#include <vector>
#include <tuple>
#include <unordered_map>

#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/GridProperty.hpp>

/*
  This class implements a container (std::unordered_map<std::string ,
  Gridproperty<T>>) of Gridproperties. Usage is as follows:

    1. Instantiate the class; passing the number of grid cells and the
       supported keywords as a list of strings to the constructor.

    2. Query the container with the supportsKeyword() and hasKeyword()
       methods.

    3. When you ask the container to get a keyword with the
       getKeyword() method it will automatically create a new
       GridProperty object if the container does not have this
       property.
*/


namespace Opm {

template <typename T>
class GridProperties {
public:
    typedef typename GridProperty<T>::SupportedKeywordInfo SupportedKeywordInfo;

    GridProperties(std::shared_ptr<const EclipseGrid> eclipseGrid , std::shared_ptr<const std::vector<SupportedKeywordInfo> > supportedKeywords) {
        m_eclipseGrid = eclipseGrid;

        for (auto iter = supportedKeywords->begin(); iter != supportedKeywords->end(); ++iter)
            m_supportedKeywords[iter->getKeywordName()] = *iter;
    }


    bool supportsKeyword(const std::string& keyword) const {
        return m_supportedKeywords.count( keyword ) > 0;
    }

    bool hasKeyword(const std::string& keyword) const {
        return m_properties.count( keyword ) > 0;
    }

    size_t size() const {
        return m_property_list.size();
    }


    std::shared_ptr<GridProperty<T> > getKeyword(const std::string& keyword) {
        if (!hasKeyword(keyword))
            addKeyword(keyword);

        return m_properties.at( keyword );
    }

    std::shared_ptr<GridProperty<T> > getKeyword(size_t index) {
        if (index < size())
            return m_property_list[index];
        else
            throw std::invalid_argument("Invalid index");
    }


    std::shared_ptr<GridProperty<T> > getInitializedKeyword(const std::string& keyword) const {
        if (hasKeyword(keyword))
            return m_properties.at( keyword );
        else {
            if (supportsKeyword(keyword))
                throw std::invalid_argument("Keyword: " + keyword + " is supported - but not initialized.");
            else
                throw std::invalid_argument("Keyword: " + keyword + " is not supported.");
        }
    }


    bool addKeyword(const std::string& keywordName) {
        if (!supportsKeyword( keywordName ))
            throw std::invalid_argument("The keyword: " + keywordName + " is not supported in this container");

        if (hasKeyword(keywordName))
            return false;
        else {
            auto supportedKeyword = m_supportedKeywords.at( keywordName );
            int nx = m_eclipseGrid->getNX();
            int ny = m_eclipseGrid->getNY();
            int nz = m_eclipseGrid->getNZ();
            std::shared_ptr<GridProperty<T> > newProperty(new GridProperty<T>(nx , ny , nz , supportedKeyword));

            m_properties.insert( std::pair<std::string , std::shared_ptr<GridProperty<T> > > ( keywordName , newProperty ));
            m_property_list.push_back( newProperty );
            return true;
        }
    }


    template <class Keyword>
    bool hasKeyword() const {
        return hasKeyword( Keyword::keywordName );
    }

    template <class Keyword>
    std::shared_ptr<GridProperty<T> > getKeyword() {
        return getKeyword( Keyword::keywordName );
    }

    template <class Keyword>
    std::shared_ptr<GridProperty<T> > getInitializedKeyword() const {
        return getInitializedKeyword( Keyword::keywordName );
    }



private:
    std::shared_ptr<const EclipseGrid> m_eclipseGrid;
    std::unordered_map<std::string, SupportedKeywordInfo> m_supportedKeywords;
    std::map<std::string , std::shared_ptr<GridProperty<T> > > m_properties;
    std::vector<std::shared_ptr<GridProperty<T> > > m_property_list;
};

}

#endif
