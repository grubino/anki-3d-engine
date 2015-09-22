R"(		<program>
			<type>frag</type>

			<includes>
				<include>shaders/MsCommonFrag.glsl</include>
			</includes>

			<inputs>
				%specularColorInput%
				%specularPowerInput%
				%diffuseColorInput%
				%normalInput%
				%subsurfaceInput%
				%emissionInput%
			</inputs>

			<operations>
				<operation>
					<id>0</id>
					<returnType>vec3</returnType>
					<function>getNormal</function>
				</operation>
				<operation>
					<id>1</id>
					<returnType>vec4</returnType>
					<function>getTangent</function>
				</operation>
				<operation>
					<id>2</id>
					<returnType>vec2</returnType>
					<function>getTextureCoord</function>
				</operation>
				%diffuseColorFunc%
				%normalFunc%
				%specularColorFunc%
				%specularPowerFunc%
				<operation>
					<id>100</id>
					<returnType>void</returnType>
					<function>writeRts</function>
					<arguments>
						<argument>%diffuseColorArg%</argument>
						<argument>%normalArg%</argument>
						<argument>%specularColorArg%</argument>
						<argument>%specularPowerArg%</argument>
						<argument>%subsurfaceArg%</argument>
						<argument>%emissionArg%</argument>
					</arguments>
				</operation>
			</operations>
		</program>)"

